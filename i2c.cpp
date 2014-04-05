#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>
using namespace std;

int main() {
	for (int f = 0; f < 100; ++f) {
		int bus = open("/dev/i2c-1", O_RDWR);
		if ( bus == -1 ){cout << "Opening Mistake" << endl;}
		int device = ioctl(bus, I2C_SLAVE, 0x68 ) ;
		if (device==-1){cout << "Ioctl Mistake" << endl;}
		char buffer[1];
		buffer[0] = 0x75;
		if (write(bus, buffer, 1) != 1) {cout << "Writing Mistake" << endl;}
		char value[1];
		if (read(bus, value, 1) != 1){cout << "Reading Mistake" << endl;}
		close(bus);
		cout << hex << int(value[0]) << endl;
	}
}
