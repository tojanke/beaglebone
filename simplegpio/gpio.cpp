#include "gpio.h"
#include <unistd.h>
#include <iostream>
using namespace std;

GPIOPin LEDPIN(60); /* P9_12 */

int main(int argc, char *argv[]) {
	cout << "Frequency:" << endl;
	int FR = 0;
	cin >> FR;
	signed int TIME = 1000000 / FR;
	LEDPIN.SetDir(OUTPUT);

	for(int i = 0; i < 20; i++){
                LEDPIN.SetValue(HIGH);
		usleep(TIME);
                LEDPIN.SetValue(LOW);
		usleep(TIME);
	}
}
