# User-facing wrapper. The real build rules live in ./Kbuild
# (the kernel build system prefers Kbuild over Makefile when both exist).

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
