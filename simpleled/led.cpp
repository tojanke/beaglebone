#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "led.h"
 
using namespace std;
 
int intervall = 50000;
int count = 60;

int main(int argc, char** argv) { 	
	cout << "Starting simple LED blink program" << endl;
 	cout << "How often per Second?" << endl;
	cin >> count;
	intervall = 1000000 / (4 * count);
  	while(1){	
 		turnoff(4);
    		turnon(1);
     		usleep(intervall);

		turnoff(1);
    		turnon(2);
    		usleep(intervall);

		turnoff(2);
    		turnon(3);
    		usleep(intervall);

		turnoff(3);
    		turnon(4);
    		usleep(intervall);
	}
 	
 	return 0;

}




