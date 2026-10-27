obj-m := renamed_rust.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 clean

.PHONY: all clean
