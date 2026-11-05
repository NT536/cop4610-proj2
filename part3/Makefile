obj-m += elevator.o
elevator-objs := src/elevator_main.o src/elevator_thread.o src/elevator_proc.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean