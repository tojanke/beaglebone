#include "led.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
	ULedInit();
	cout << "Starting to blink!" << endl;
	for (int s = 1; s <= 20; s++) {
		cout << "Blinking for 8 Seconds with " << s << " BPS" << endl;
		blink(s, 8);
	}
	return 0;
}


	

