#include "beaglebone.h"
#include <iostream>
#include <unistd.h>
using namespace std;

int main(int argc, char** argv) {
	PWM::PIN P4(4, 1000);
	cout << "Initialized PWM Pin 4 - EHRPWM1B" << endl;
	P4.Enable();
	for (int pp = 1; pp <= 1000; pp++){
		P4.SetDuty(pp);
		usleep(1000);
	}
	return 0;
}
