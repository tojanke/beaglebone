#include "beaglebone.h"
#include <iostream>
#include <unistd.h>
using namespace std;

int main(int argc, char** argv) {
	PWM::PIN P4(4, 1000);
	cout << "Initialized PWM Pin 4 - EHRPWM1B" << endl;
	GPIO::PIN G60(60, OUTPUT);
	cout << "Initialized GPIO Pin 60 - GPIO1_28" << endl;
	USERLED::ULedInit();
	cout << "Initialized User LEDs 1-4" << endl;
	cout << "Testing" << endl;
	USERLED::LED uLED[] = {0, 1, 2, 3};
	for (int f = 0; f < 4; ++f) {
		for (int l1 = 0; l1 < 4; l1++) {
			uLED[l1].on();
		}
		P4.Enable();
		for (int p1 = 1; p1 <= 1000; p1++) {
			P4.SetDuty(p1);
			usleep(1000);	
		}
		P4.Disable();
		for (int l2 = 0; l2 < 4; l2++) {
			uLED[l2].off();
		}
		G60.SetValue(HIGH);
		usleep(1000000);
		G60.SetValue(LOW);
		for (int l3 = 0; l3 < 4; l3++) {
			uLED[l3].on();
		}
		P4.Enable();
		for (int p2 = 1000; p2 > 0; --p2) {
			P4.SetDuty(p2);
			usleep(1000);
		}
		P4.Disable();
		for (int l4 = 0; l4 < 4; l4++) {
			uLED[l4].off();
		}
		G60.SetValue(HIGH);
		usleep(1000000);
		G60.SetValue(LOW);
	}
	cout << "Full Test Completed" << endl;
#if LOG_OUTPUT
	LOG::Write("INFO", "compile.cpp::main", "Testing Log Function");
#endif
	return 0;
}
