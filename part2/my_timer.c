#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathaniel Thompson");
MODULE_DESCRIPTION("Timer Module for class COP4610");

#define ENTRY_NAME "my_timer"
#define PERMS 0644
#define PARENT NULL

static struct proc_dir_entry* timer_entry;
bool timer_initialized = false;
struct timespec64 ts_past;

static ssize_t timer_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    struct timespec64 ts_now;
    char buf[256];
    int len = 0;

    ktime_get_real_ts64(&ts_now);

    if(timer_initialized)
    {
        //difference between current and past times
        struct timespec64 diff;
        diff.tv_sec = ts_now.tv_sec - ts_past.tv_sec;
        diff.tv_nsec = ts_now.tv_nsec - ts_past.tv_nsec;
        if (diff.tv_nsec < 0) {
            printk(KERN_ERR "Time difference calculation error\n");
        }
        len = snprintf(buf, sizeof(buf), "current time: %lld\n", (long long)ts_now.tv_sec);
        len += snprintf(buf + len, sizeof(buf) - len, "time difference: %lld.%09ld\n", (long long)diff.tv_sec, diff.tv_nsec);
    }
    else
    {
        len = snprintf(buf, sizeof(buf), "current time: %lld\n", (long long)ts_now.tv_sec);
    }

    if (!timer_initialized)
    {
        timer_initialized = true;
        ts_past = ts_now;
    }
    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops timer_fops = {
    .proc_read = timer_read,
};

//need to create a proc entry for the timer
static int __init my_timer_init(void)
{
    timer_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &timer_fops);
    if (!timer_entry) {
        printk(KERN_ERR "Failed to create proc entry\n");
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