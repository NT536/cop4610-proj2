# Kernel Made Pet Elevator

[Description]

## Group Members

- **Malachi Davey**: mld24d@fsu.edu
- **Nathaniel Thompson**: ntt23@fsu.edu
- **Akshat Thapliyal**: at24e@fsu.edu

## Division of Labor

### Part 1: System-Call Tracing

- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

### Part 2: Timer Kernel Module

- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson, Malachi Davey

### Part 3a: Add System Calls

- **Responsibilities**: Added system calls 548-550 (`start_elevator`, `issue_request`,
  `stop_elevator`) to the kernel as thin wrappers that forward to function pointers the
  elevator module sets on load (`part3/syscalls.c`, `part3/scripts/add_syscalls.sh`).
- **Assigned to**: Akshat Thapliyal

### Part 3b: Compile and Install Kernel

- **Responsibilities**: Compile and install linux-6.16.12 with the new system calls on the
  VM, then verify them with the `system-calls-test` programs (see Part 3 Kernel Setup).
- **Assigned to**: Akshat Thapliyal

### Part 3c: Test System Calls

- **Responsibilities**: [Description]
- **Assigned to**: Malachi Davey

### Part 3d: Implement Elevator

- **Responsibilities**: [Description]
- **Assigned to**: Malachi Davey

### Part 3e: Write to the Proc File

- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

### Part 3f: Test Elevator

- **Responsibilities**: [Description]
- **Assigned to**: Nathaniel Thompson

## File Listing

```
Proj2COP4610/
│
├── part1/
│ ├── empty.c
│ ├── empty.trace
│ ├── part1.c
| └── part1.trace
├── part2/
├── part3/
│ ├── syscalls.c
│ └── scripts/
│   └── add_syscalls.sh
├── README.md
└── Makefile
```

## How to Compile & Execute

### Requirements

- **Compiler**: gcc is the required compiler, as the work is in c.
- **Dependencies**: List any libraries or frameworks necessary (rust only).

### Compilation

For a C/C++ example:

```bash
make
```

This will build the executable in side the bin folder

### Execution

```bash
make run
```

This will run the program ...

## Part 3 Kernel Setup (one time, on the VM)

The system calls are compiled into the kernel, so this is done once. The elevator logic lives
in the `elevator` module and can be rebuilt without touching the kernel again.

```bash
sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex bc dwarves -y
cd ~ && wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.16.12.tar.xz
sudo tar -xf linux-6.16.12.tar.xz -C /usr/src
sudo chown -R $USER:$USER /usr/src/linux-6.16.12
ln -s /usr/src/linux-6.16.12 ~/linux-6.16.12
~/cop4610-proj2/part3/scripts/add_syscalls.sh ~/linux-6.16.12
cd ~/linux-6.16.12
cp /boot/config-$(uname -r) .config
make olddefconfig
make menuconfig            # no changes needed: Save, then Exit
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
make -j$(nproc)            # press Enter at any certificate prompts
sudo make modules_install -j$(nproc)
sudo make headers_install
sudo make install
sudo update-grub
sudo reboot
uname -r                   # should print 6.16.12
```

Checking the system calls after reboot (from the starter `part3/tests/system-calls-test/`):

```bash
make
sudo insmod syscheck.ko
./test-syscalls
sudo rmmod syscheck
```

## Development Log

Each member records their contributions here.

### Malachi Davey

| Date       | Work Completed / Notes |
| ---------- | ---------------------- |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |
| YYYY-MM-DD | [Description of task]  |

### Nathaniel Thompson

<<<<<<< HEAD
| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-10-23 | Created README.md document, along with completing part1 of the project, before moving onto research and the creation of part2  |
| 2026-10-29 | Created makefile and did some adjustment to the my_timer.c file. Added .gitignore file.   |
| 2026-10-30 | Compiled and installed the kernel system calls into the Linux VM. Was stalled part way through. |
=======
| Date       | Work Completed / Notes                                                                                                        |
| ---------- | ----------------------------------------------------------------------------------------------------------------------------- |
| 2026-10-23 | Created README.md document, along with completing part1 of the project, before moving onto research and the creation of part2 |
| YYYY-MM-DD | Created makefile and did some adjustment to the my_timer.c file. Added .gitignore file.                                       |
| YYYY-MM-DD | [Description of task]                                                                                                         |
>>>>>>> cc9a0b835869faa7c7fd9fb4bd459dec3f574395

### Akshat Thapliyal

| Date       | Work Completed / Notes                                                                                                                                                                                                        |
| ---------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 2026-10-29 | Reviewed the kernel compilation slides and starter code; planned how to add system calls 548-550 using stub function pointers so the elevator module can be rebuilt without recompiling the kernel.                           |
| 2026-10-29 | Wrote part3/syscalls.c (start_elevator, issue_request, stop_elevator wrappers) and part3/scripts/add_syscalls.sh to apply the kernel edits (syscall_64.tbl, syscalls.h, top-level Makefile, syscalls/ folder).                |
| 2026-10-29 | Test-built linux-6.16.12 with the new system calls and confirmed with the starter test-syscalls program that all 3 calls work once syscheck.ko is loaded. Added kernel setup steps and Part 3a/3b descriptions to the README. |
| YYYY-MM-DD | Compiled and installed linux-6.16.12 on the VM, rebooted, verified uname -r and ran test-syscalls.                                                                                                                            |

## Meetings

Document in-person meetings, their purpose, and what was discussed.

| Date | Attendees  | Topics Discussed          | Outcomes / Decisions |
| ---- | ---------- | ------------------------- | -------------------- |
| Date | Name, Name | Topics that are discussed | Decions decisions    |

## Bugs

- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Extra Credit

- **Extra Credit 1**: [Extra Credit Option]
- **Extra Credit 2**: [Extra Credit Option]
- **Extra Credit 3**: [Extra Credit Option]

## Considerations

[Description]
