#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathaniel Thompson, Malachi Davey");     // FIX: both authors
MODULE_DESCRIPTION("Timer Module for class COP4610");

#define ENTRY_NAME "timer"      // FIX: spec requires /proc/timer 
#define PERMS 0444              // FIX: read-only entry 
#define PARENT NULL

static struct proc_dir_entry* timer_entry;
static bool timer_initialized = false;  // FIX: static, don't leak into kernel namespace 
static struct timespec64 ts_past;       // FIX: static 

static ssize_t timer_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    struct timespec64 ts_now;
    char buf[256];
    int len = 0;

    // FIX: cat calls read again to check for EOF; return before touching ts_past 
    if (*ppos > 0)
        return 0;

    ktime_get_real_ts64(&ts_now);

    // FIX: current time now includes nanoseconds
    len = snprintf(buf, sizeof(buf), "current time: %lld.%09ld\n",
                   (long long)ts_now.tv_sec, ts_now.tv_nsec);

    if (timer_initialized)
    {
        //difference between current and past times
        struct timespec64 diff;
        diff.tv_sec = ts_now.tv_sec - ts_past.tv_sec;
        diff.tv_nsec = ts_now.tv_nsec - ts_past.tv_nsec;
        if (diff.tv_nsec < 0) {
            // FIX: borrow a second instead of printing an error (printf doesn't exist in kernel) 
            diff.tv_sec -= 1;
            diff.tv_nsec += NSEC_PER_SEC;
        }
        /* FIX: label matches spec */
        len += snprintf(buf + len, sizeof(buf) - len, "elapsed time: %lld.%09ld\n",
                        (long long)diff.tv_sec, diff.tv_nsec);
    }

    // FIX: update on every read so elapsed is since the LAST call, not the first
    timer_initialized = true;
    ts_past = ts_now;

    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // copies to user space
}

static const struct proc_ops timer_fops = {
    .proc_read = timer_read,
};

//need to create a proc entry for the timer
static int __init my_timer_init(void)
{
    timer_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &timer_fops);
    if (!timer_entry) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit my_timer_exit(void)
{
    proc_remove(timer_entry);
}

module_init(my_timer_init);
module_exit(my_timer_exit);