#include "pwm.h"
#include <iostream>
#include <unistd.h>
using namespace std;

int main(int argc, char** argv) {
	PWM::PIN P4(4, 1000);
	cout << "Initialized PWM Pin 4 - EHRPWM1B" << endl;
	P4.Enable();
	cout << "Enabled Pin 4" << endl;
	cout << "Increasing Duty Cycle to 100%" << endl;
	for (int i = 1; i <= 1000; i += 10) {
		P4.SetDuty(i);
		usleep(10000);
	}
	cout << "PWM Test Completed" << endl;
	P4.Disable();
	cout << "Disabled Pin 4" << endl;
	GPIO::PIN G60(60, OUTPUT);
	cout << "Initialized GPIO Pin 60 - GPIO1_28" << endl;
	cout << "Turning it on for 1 Second" << endl;
	G60.SetValue(HIGH);
	usleep(1000000);
	G60.SetValue(LOW);
	cout << "Switched Off" << endl;
	cout << "GPIO Test Completed" << endl;
	USERLED::ULedInit();
	cout << "Initialized User LEDs 1-4" << endl;
	cout << "Blinking each LED once" << endl;
	USERLED::blink(1, 4);
	cout << "ULED Test Completed" << endl;
	return 0;
}
