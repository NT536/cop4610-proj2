/*
 * /proc/elevator: prints the elevator state, its riders, and each floor's
 * waiting line in the format from the spec.
 */

#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "elevator.h"

#define ENTRY_NAME "elevator"
#define PERMS 0444
#define PROC_BUF_SIZE 10000     /* spec: proc output stays under 10,000 chars */

static struct proc_dir_entry *elevator_entry;

static const char *state_name(enum elevator_state state)
{
	switch (state) {
	case OFFLINE: return "OFFLINE";
	case IDLE:    return "IDLE";
	case LOADING: return "LOADING";
	case UP:      return "UP";
	case DOWN:    return "DOWN";
	default:      return "UNKNOWN";
	}
}

/* append "C5 " style entries for every pet in a list; caller holds lock */
static int print_pets(char *buf, int len, struct list_head *head)
{
	struct pet *pet;

	list_for_each_entry(pet, head, list)
		len += scnprintf(buf + len, PROC_BUF_SIZE - len, " %c%d",
				 pet_letter(pet->type), pet->dest_floor);
	return len;
}

static ssize_t elevator_read(struct file *file, char __user *ubuf,
			     size_t count, loff_t *ppos)
{
	char *buf;
	int len = 0;
	int floor;
	ssize_t ret;

	buf = kmalloc(PROC_BUF_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	mutex_lock(&elevator_lock);

	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Elevator state: %s\n",
			 state_name(elevator.state));
	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Current floor: %d\n",
			 elevator.current_floor);
	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Current load: %d lbs\n",
			 elevator.load);
	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "Elevator status:");
	len = print_pets(buf, len, &elevator.pets);
	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n\n");

	for (floor = MAX_FLOOR; floor >= MIN_FLOOR; floor--) {
		int idx = floor_index(floor);

		len += scnprintf(buf + len, PROC_BUF_SIZE - len, "[%c] Floor %d: %d",
				 floor == elevator.current_floor ? '*' : ' ',
				 floor, building.waiting[idx]);
		len = print_pets(buf, len, &building.floors[idx]);
		len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\n");
	}

	len += scnprintf(buf + len, PROC_BUF_SIZE - len, "\nNumber of pets: %d\n",
			 elevator.num_pets);
	len += scnprintf(buf + len, PROC_BUF_SIZE - len,
			 "Number of pets waiting: %d\n", building.total_waiting);
	len += scnprintf(buf + len, PROC_BUF_SIZE - len,
			 "Number of pets serviced: %d\n", building.total_serviced);

	mutex_unlock(&elevator_lock);

	/* handles *ppos, so cat's second read returns 0 */
	ret = simple_read_from_buffer(ubuf, count, ppos, buf, len);
	kfree(buf);
	return ret;
}

static const struct proc_ops elevator_fops = {
	.proc_read = elevator_read,
};

int elevator_proc_init(void)
{
	elevator_entry = proc_create(ENTRY_NAME, PERMS, NULL, &elevator_fops);
	return elevator_entry ? 0 : -ENOMEM;
}

void elevator_proc_exit(void)
{
	proc_remove(elevator_entry);
}