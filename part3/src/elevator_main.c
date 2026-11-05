/*
 * Module setup and the three system call handlers. The syscalls built into
 * the kernel (part3/syscalls.c) forward to the STUB_ pointers, which we point
 * at our functions on insmod and clear on rmmod.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malachi Davey, Nathaniel Thompson, Akshat Thapliyal");
MODULE_DESCRIPTION("Pet elevator kernel module for COP4610 Project 2");

/* exported by part3/syscalls.c inside the kernel */
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

struct elevator elevator;
struct building building;
DEFINE_MUTEX(elevator_lock);

static struct task_struct *elevator_thread;

int pet_weight(int type)
{
	switch (type) {
	case CHIHUAHUA: return 3;
	case PUG:       return 14;
	case PUGHUAHUA: return 10;
	case DACHSHUND: return 16;
	default:        return 0;
	}
}

char pet_letter(int type)
{
	switch (type) {
	case CHIHUAHUA: return 'C';
	case PUG:       return 'P';
	case PUGHUAHUA: return 'H';
	case DACHSHUND: return 'D';
	default:        return '?';
	}
}

/* 548: returns 1 if already running, 0 on a successful start */
static int start_elevator(void)
{
	int ret = 0;

	mutex_lock(&elevator_lock);
	if (elevator.state != OFFLINE) {
		ret = 1;
	} else {
		elevator.state = IDLE;
		elevator.current_floor = MIN_FLOOR;
		elevator.direction = 1;
		elevator.num_pets = 0;
		elevator.load = 0;
		elevator.deactivating = false;
	}
	mutex_unlock(&elevator_lock);
	return ret;
}

/* 549: returns 1 for an invalid request, 0 once the pet is queued */
static int issue_request(int start_floor, int destination_floor, int type)
{
	struct pet *pet;
	int idx;

	if (start_floor < MIN_FLOOR || start_floor > MAX_FLOOR ||
	    destination_floor < MIN_FLOOR || destination_floor > MAX_FLOOR ||
	    start_floor == destination_floor ||
	    type < 0 || type >= NUM_PET_TYPES)
		return 1;

	/* allocate before locking: kmalloc can sleep */
	pet = kmalloc(sizeof(*pet), GFP_KERNEL);
	if (!pet)
		return -ENOMEM;

	pet->type = type;
	pet->start_floor = start_floor;
	pet->dest_floor = destination_floor;
	pet->weight = pet_weight(type);

	idx = floor_index(start_floor);
	mutex_lock(&elevator_lock);
	list_add_tail(&pet->list, &building.floors[idx]);   /* tail = FIFO */
	building.waiting[idx]++;
	building.total_waiting++;
	mutex_unlock(&elevator_lock);
	return 0;
}

/*
 * 550: stop picking up pets, let the kthread drop off everyone riding, then
 * it sets OFFLINE. Returns 1 if a stop is already in progress, 0 otherwise.
 */
static int stop_elevator(void)
{
	int ret = 0;

	mutex_lock(&elevator_lock);
	if (elevator.deactivating)
		ret = 1;
	else if (elevator.state != OFFLINE)
		elevator.deactivating = true;
	mutex_unlock(&elevator_lock);
	return ret;
}

/* kfree every pet in a list; caller holds elevator_lock */
static void free_pet_list(struct list_head *head)
{
	struct pet *pet, *tmp;

	/* _safe version because we free the node we're standing on */
	list_for_each_entry_safe(pet, tmp, head, list) {
		list_del(&pet->list);
		kfree(pet);
	}
}

static int __init elevator_init(void)
{
	int i, ret;

	elevator.state = OFFLINE;
	elevator.current_floor = MIN_FLOOR;
	elevator.direction = 1;
	elevator.num_pets = 0;
	elevator.load = 0;
	elevator.deactivating = false;
	INIT_LIST_HEAD(&elevator.pets);

	for (i = 0; i < NUM_FLOORS; i++) {
		INIT_LIST_HEAD(&building.floors[i]);
		building.waiting[i] = 0;
	}
	building.total_waiting = 0;
	building.total_serviced = 0;

	ret = elevator_proc_init();
	if (ret)
		return ret;

	elevator_thread = kthread_run(elevator_thread_fn, NULL, "elevator_thread");
	if (IS_ERR(elevator_thread)) {
		elevator_proc_exit();
		return PTR_ERR(elevator_thread);
	}

	/* only hook up the syscalls once everything they touch exists */
	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;
	return 0;
}

static void __exit elevator_exit(void)
{
	int i;

	/* unhook first so no new syscalls reach the module */
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	kthread_stop(elevator_thread);   /* waits for the loop to return */
	elevator_proc_exit();

	mutex_lock(&elevator_lock);
	free_pet_list(&elevator.pets);
	for (i = 0; i < NUM_FLOORS; i++)
		free_pet_list(&building.floors[i]);
	mutex_unlock(&elevator_lock);
}

module_init(elevator_init);
module_exit(elevator_exit);