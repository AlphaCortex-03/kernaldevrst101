/*
 * user_app.c - the USERSPACE side of lesson 011.
 *
 * A normal C program that opens /dev/mydevice and talks to the kernel
 * module: it writes data, then sends ioctl() commands.
 *
 * Build: gcc user_app.c -o user_app   (the Makefile does this for you)
 * Run  : ./user_app                   (module must be loaded)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* Must match main.c EXACTLY - same magic, same numbers. */
#define IOCTL_MAGIC 'g'
#define IOCTL_CLEAR	_IO(IOCTL_MAGIC, 1)
#define IOCTL_GET_SIZE	_IOR(IOCTL_MAGIC, 2, int)
#define IOCTL_SET_VALUE	_IOW(IOCTL_MAGIC, 3, int)

int main(void)
{
	int fd, ret, size = 0;
	const char *msg = "hello ioctl";

	fd = open("/dev/mydevice", O_RDWR);
	if (fd < 0) {
		perror("open /dev/mydevice (is the module loaded? try: sudo insmod myioctl.ko)");
		return 1;
	}

	/* 1. put some data into the kernel buffer (plain write) */
	ret = write(fd, msg, strlen(msg));
	if (ret < 0) {
		perror("write");
		close(fd);
		return 1;
	}
	printf("wrote %d bytes to the device\n", ret);

	/* 2. IOCTL_GET_SIZE: kernel sends an int OUT into `size` */
	if (ioctl(fd, IOCTL_GET_SIZE, &size) < 0) {
		perror("ioctl GET_SIZE");
		close(fd);
		return 1;
	}
	printf("kernel says buffer size = %d\n", size);

	/* 3. IOCTL_SET_VALUE: we send an int IN to the kernel */
	int value = 99;
	if (ioctl(fd, IOCTL_SET_VALUE, &value) < 0) {
		perror("ioctl SET_VALUE");
		close(fd);
		return 1;
	}
	printf("sent value %d to the kernel\n", value);

	/* 4. IOCTL_CLEAR: command with no data at all */
	if (ioctl(fd, IOCTL_CLEAR) < 0) {
		perror("ioctl CLEAR");
		close(fd);
		return 1;
	}
	printf("sent CLEAR command\n");

	/* 5. prove the buffer is now empty */
	size = -1;
	if (ioctl(fd, IOCTL_GET_SIZE, &size) < 0) {
		perror("ioctl GET_SIZE");
		close(fd);
		return 1;
	}
	printf("size after clear = %d\n", size);

	close(fd);
	return 0;
}
