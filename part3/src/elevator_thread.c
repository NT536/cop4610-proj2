/*
 * The kthread that runs the elevator. Each pass it locks the shared state,
 * decides what to do (load/unload, move one floor, go idle, or go offline),
 * unlocks, and only then sleeps, so the syscalls and proc file never wait
 * behind a 1-2 second sleep.
 *
 * Scheduling is LOOK: keep moving in the current direction while there is
 * anything to do that way, otherwise turn around, otherwise go idle.
 */

#include <linux/delay.h>
#include <linux/slab.h>
#include "elevator.h"

/* does a riding pet want to get off here? caller holds elevator_lock */
static bool pet_to_unload(void)
{
	struct pet *pet;

	list_for_each_entry(pet, &elevator.pets, list)
		if (pet->dest_floor == elevator.current_floor)
			return true;
	return false;
}

/* can the pet at the front of this floor's line board? caller holds lock */
static bool pet_to_load(void)
{
	struct list_head *line = &building.floors[floor_index(elevator.current_floor)];
	struct pet *front;

	if (elevator.deactivating || list_empty(line))
		return false;
	front = list_first_entry(line, struct pet, list);
	return elevator.num_pets < MAX_PETS &&
	       elevator.load + front->weight <= MAX_WEIGHT;
}

/* drop off riders for this floor, then board FIFO; caller holds lock */
static void unload_and_load(void)
{
	int idx = floor_index(elevator.current_floor);
	struct list_head *line = &building.floors[idx];
	struct pet *pet, *tmp;

	list_for_each_entry_safe(pet, tmp, &elevator.pets, list) {
		if (pet->dest_floor == elevator.current_floor) {
			list_del(&pet->list);
			elevator.num_pets--;
			elevator.load -= pet->weight;
			building.total_serviced++;
			kfree(pet);
		}
	}

	if (elevator.deactivating)
		return;

	/* strict FIFO: stop at the first pet that doesn't fit */
	while (!list_empty(line)) {
		pet = list_first_entry(line, struct pet, list);
		if (elevator.num_pets >= MAX_PETS ||
		    elevator.load + pet->weight > MAX_WEIGHT)
			break;
		list_move_tail(&pet->list, &elevator.pets);
		elevator.num_pets++;
		elevator.load += pet->weight;
		building.waiting[idx]--;
		building.total_waiting--;
	}
}

/* is there a rider destination or waiting pet beyond us in dir? holds lock */
static bool work_in_direction(int dir)
{
	struct pet *pet;
	int floor;

	list_for_each_entry(pet, &elevator.pets, list)
		if ((pet->dest_floor - elevator.current_floor) * dir > 0)
			return true;

	if (elevator.deactivating)
		return false;

	for (floor = elevator.current_floor + dir;
	     floor >= MIN_FLOOR && floor <= MAX_FLOOR; floor += dir)
		if (building.waiting[floor_index(floor)] > 0)
			return true;
	return false;
}

int elevator_thread_fn(void *data)
{
	while (!kthread_should_stop()) {
		mutex_lock(&elevator_lock);

		if (elevator.state == OFFLINE) {
			mutex_unlock(&elevator_lock);
			msleep(100);
			continue;
		}

		/* 1. stopped on a floor with someone to drop off or pick up */
		if (pet_to_unload() || pet_to_load()) {
			elevator.state = LOADING;
			mutex_unlock(&elevator_lock);
			ssleep(LOAD_TIME);

			mutex_lock(&elevator_lock);
			unload_and_load();
			mutex_unlock(&elevator_lock);
			continue;
		}

		/* 2. stop_elevator() was called and everyone is off */
		if (elevator.deactivating && elevator.num_pets == 0) {
			elevator.state = OFFLINE;
			elevator.deactivating = false;
			mutex_unlock(&elevator_lock);
			continue;
		}

		/* 3. pick a direction (LOOK), or idle if nothing to do */
		if (!work_in_direction(elevator.direction)) {
			if (work_in_direction(-elevator.direction)) {
				elevator.direction = -elevator.direction;
			} else {
				elevator.state = IDLE;
				mutex_unlock(&elevator_lock);
				msleep(100);
				continue;
			}
		}

		/* 4. move one floor; the floor number changes after the trip */
		elevator.state = (elevator.direction > 0) ? UP : DOWN;
		mutex_unlock(&elevator_lock);
		ssleep(MOVE_TIME);

		mutex_lock(&elevator_lock);
		elevator.current_floor += elevator.direction;
		mutex_unlock(&elevator_lock);
	}
	return 0;
}