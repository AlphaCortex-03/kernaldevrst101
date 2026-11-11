obj-m := myioctl.o
myioctl-objs := main.o

all: user_app
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# userspace test program (plain gcc, not the kernel build system)
user_app:
	gcc user_app.c -o user_app

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user_app

.PHONY: all clean user_app
