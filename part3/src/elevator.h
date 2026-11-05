#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>

/* building and elevator limits from the spec */
#define MIN_FLOOR 1
#define MAX_FLOOR 5
#define NUM_FLOORS (MAX_FLOOR - MIN_FLOOR + 1)
#define MAX_PETS 5
#define MAX_WEIGHT 50

/* wait times from the spec, in seconds */
#define MOVE_TIME 2
#define LOAD_TIME 1

/* pet types, matching the type argument of issue_request() */
enum pet_type {
	CHIHUAHUA = 0,
	PUG = 1,
	PUGHUAHUA = 2,
	DACHSHUND = 3,
	NUM_PET_TYPES
};

enum elevator_state {
	OFFLINE,
	IDLE,
	LOADING,
	UP,
	DOWN
};

/* one pet, allocated with kmalloc and linked into a floor or the elevator */
struct pet {
	int type;
	int start_floor;
	int dest_floor;
	int weight;
	struct list_head list;
};

/* everything the kthread, the syscalls and the proc file share */
struct elevator {
	enum elevator_state state;
	int current_floor;
	int direction;              /* +1 going up, -1 going down */
	int num_pets;               /* pets currently riding */
	int load;                   /* current weight in lbs */
	bool deactivating;          /* stop_elevator() called, still unloading */
	struct list_head pets;      /* pets riding, in boarding order */
};

struct building {
	struct list_head floors[NUM_FLOORS];    /* FIFO queue of waiting pets per floor */
	int waiting[NUM_FLOORS];                /* pets waiting on each floor */
	int total_waiting;
	int total_serviced;
};

/* defined in elevator_main.c, guarded by elevator_lock */
extern struct elevator elevator;
extern struct building building;
extern struct mutex elevator_lock;

/* elevator_main.c */
int pet_weight(int type);
char pet_letter(int type);

/* elevator_thread.c */
int elevator_thread_fn(void *data);

/* elevator_proc.c */
int elevator_proc_init(void);
void elevator_proc_exit(void);

/* floor numbers are 1-5, array indexes are 0-4 */
static inline int floor_index(int floor)
{
	return floor - MIN_FLOOR;
}

#endif