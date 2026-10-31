/*
 * So below are our 3 elevator system calls (548, 549, 550). This file gets
 * built right into the kernel, which is why we only need to compile it one time.
 * None of the actual elevator code lives here. Each syscall just calls a
 * function pointer (a "stub"). When we insmod the elevator module, it takes
 * the stubs and points them at its own functions. Then when we rmmod it, they
 * go back to NULL, so we can rebuild the module as many times as we want
 * without recompiling the whole kernel.
 */

#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

/*
 * The stubs. They start out NULL, meaning no elevator module is loaded yet.
 * Their signatures match what the assignment asks for.
 */
int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;

/*
 * Modules can't see kernel variables unless they're exported, so this is
 * what lets the elevator module (and the test module) set the stubs.
 * Without it, insmod would fail with an "undefined symbol" error.
 */
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

/*
 * SYSCALL_DEFINEx is a kernel macro that creates the real syscall function.
 * The number is how many arguments it takes, and the args are written as
 * "type, name" pairs. The names have to match the ones in syscall_64.tbl.
 *
 * Every syscall below works the same way: if the module is loaded, pass
 * the call along to it. If not, return -ENOSYS ("not implemented") so we
 * don't jump to a NULL pointer and crash the kernel.
 */

/* 548: turns the elevator on (starts IDLE on floor 1 with no pets) */
SYSCALL_DEFINE0(start_elevator)
{
	if (STUB_start_elevator != NULL)
		return STUB_start_elevator();
	return -ENOSYS;
}

/*
 * 549: adds a pet waiting on start_floor that wants to go to destination_floor.
 * type is 0 = Chihuahua, 1 = Pug, 2 = Pughuahua, 3 = Dachshund
 */
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type)
{
	if (STUB_issue_request != NULL)
		return STUB_issue_request(start_floor, destination_floor, type);
	return -ENOSYS;
}

/*
 * 550: shuts the elevator down. It stops picking up new pets, drops off
 * whoever's still riding, and then goes OFFLINE
 */
SYSCALL_DEFINE0(stop_elevator)
{
	if (STUB_stop_elevator != NULL)
		return STUB_stop_elevator();
	return -ENOSYS;
}
