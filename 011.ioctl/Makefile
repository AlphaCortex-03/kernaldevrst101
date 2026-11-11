# User-facing wrapper. The real build rules live in ./Kbuild

all: user_app
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# The userspace test program (plain gcc, not the kernel build system).
user_app:
	gcc user_app.c -o user_app

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user_app
