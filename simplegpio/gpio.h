#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

enum PIN_DIRECTION{
	INPUT=0,
	OUTPUT=1
};

enum PIN_VALUE{
	LOW=0,
	HIGH=1,
	FAIL=13
};

class GPIOPin {
public:
	GPIOPin( unsigned int gpio ) { 
		ID = gpio;
		int fd, len;
		char buf[MAX_BUF];

		fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
		if (fd < 0) {
			perror("gpio/export");
		}

		len = snprintf(buf, sizeof(buf), "%d", ID);
		write(fd, buf, len);
		close(fd);
	}
	~GPIOPin() {
		int fd, len;
		char buf[MAX_BUF];
		fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
		if (fd < 0) { perror("gpio/export"); }
		len = snprintf(buf, sizeof(buf), "%d", ID);
		write(fd, buf, len);
		close(fd);
	}
	PIN_VALUE GetValue() {
		int fd;
		char buf[MAX_BUF];
		char ch;
		snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", ID);
		fd = open(buf, O_RDONLY);
		if (fd < 0) {
			perror("gpio/get-value");
			return FAIL;
		}
		read(fd, &ch, 1);	
		if (ch != '0') {
			return HIGH;
		} else {
			return LOW;
		}
		close(fd);
	}
	int SetValue( PIN_VALUE value ) {
		int fd;
		char buf[MAX_BUF];

		snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", ID);

		fd = open(buf, O_WRONLY);
		if (fd < 0) {
			perror("gpio/set-value");
			return fd;
		}

		if (value==LOW)
			write(fd, "0", 2);
		else
			write(fd, "1", 2);

		close(fd);
		return 0;
	}
	int SetDir( PIN_DIRECTION flag ) {
		int fd;
		char buf[MAX_BUF];

		snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", ID);

		fd = open(buf, O_WRONLY);
		if (fd < 0) {
			perror("gpio/direction");
			return fd;
		}

		if (flag == OUTPUT)
			write(fd, "out", 4);
		else
			write(fd, "in", 3);

		close(fd);
		return 0;
	}
private:
	unsigned int ID;
};
