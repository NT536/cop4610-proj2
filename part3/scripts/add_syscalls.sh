#!/bin/bash

# So instead of manually editing 5 different kernel files, this script does it for us!
# We just point it at the kernel source folder: ./add_syscalls.sh ~/linux-6.16.12
# In case we run it twice by accident, the script skips anything it already did,
# so we don't end up with duplicate lines.

# bail out right away if any command fails
set -euo pipefail

# the kernel folder we were given, our part3 folder (where syscalls.c is),
# and the 3 kernel files we need to edit
KDIR="${1:?usage: $0 <path to kernel source>}"
PART3="$(cd "$(dirname "$0")/.." && pwd)"
TBL="$KDIR/arch/x86/entry/syscalls/syscall_64.tbl"
HDR="$KDIR/include/linux/syscalls.h"
MK="$KDIR/Makefile"

# sanity check: make sure this is actually a kernel folder before we start
# changing things
for f in "$TBL" "$HDR" "$MK"; do
	[ -f "$f" ] || { echo "missing $f - is $KDIR a kernel tree?"; exit 1; }
done

# step 1: make the syscalls/ folder, copy our code in, and give it a
# one-line Makefile. obj-y means "build this into the kernel," as opposed
# to obj-m, which would make it a separate module
mkdir -p "$KDIR/syscalls"
cp "$PART3/syscalls.c" "$KDIR/syscalls/syscalls.c"
echo "obj-y := syscalls.o" > "$KDIR/syscalls/Makefile"
echo "[ok] syscalls/syscalls.c, syscalls/Makefile"

# step 2: add our 3 calls to the syscall table. This is how the kernel
# knows that syscall number 548 means sys_start_elevator, and so on.
# 512-547 are reserved for old x32 stuff, so 548 is the first free number.
# "common" means 64-bit and x32 programs can both use it
if grep -q "sys_start_elevator" "$TBL"; then
	echo "[skip] syscall_64.tbl already has elevator calls"
else
	printf '%s\n' \
		"548	common	start_elevator		sys_start_elevator" \
		"549	common	issue_request		sys_issue_request" \
		"550	common	stop_elevator		sys_stop_elevator" >> "$TBL"
	echo "[ok] syscall_64.tbl"
fi

# step 3: add the function prototypes to syscalls.h. They have to go
# before the last #endif in the file or they'd be outside the header guard
if grep -q "sys_start_elevator" "$HDR"; then
	echo "[skip] syscalls.h already has elevator prototypes"
else
	last_endif=$(grep -n '^#endif' "$HDR" | tail -1 | cut -d: -f1)
	sed -i "$((last_endif - 1))r /dev/stdin" "$HDR" <<'EOF'

asmlinkage long sys_start_elevator(void);
asmlinkage long sys_issue_request(int start_floor, int destination_floor, int type);
asmlinkage long sys_stop_elevator(void);
EOF
	echo "[ok] include/linux/syscalls.h"
fi

# step 4: tell the main kernel Makefile to build our syscalls/ folder.
# core-y is the list of folders that get compiled straight into the kernel,
# so "core-y :=" becomes "core-y := syscalls/"
if grep -qE '^core-y[[:space:]]*:=.*syscalls/' "$MK"; then
	echo "[skip] Makefile core-y already has syscalls/"
else
	sed -i -E 's|^(core-y[[:space:]]*:=.*)$|\1 syscalls/|' "$MK"
	grep -qE '^core-y[[:space:]]*:=.*syscalls/' "$MK" ||
		{ echo "could not find 'core-y :=' in $MK"; exit 1; }
	echo "[ok] Makefile core-y"
fi

echo "Done. Next: see the kernel build steps in README.md (Part 3)."
