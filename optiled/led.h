#include <unistd.h>
#include <stdio.h>
#include <iostream>
using namespace std;

char *LED[] = {	"/sys/class/leds/beaglebone:green:usr0/brightness", 
		"/sys/class/leds/beaglebone:green:usr1/brightness", 
		"/sys/class/leds/beaglebone:green:usr2/brightness", 
		"/sys/class/leds/beaglebone:green:usr3/brightness"};



class UserLed {
public:
	UserLed() { 
		index = 0;
		Brightness = LED[index];
	}
	UserLed( int custom ) { 
		index = custom;
		Brightness = LED[index];	
	}
	~UserLed() { }
	int on( ) { 
		FILE *Handle = NULL;
		if((Handle = fopen(Brightness, "r+")) != NULL){
			fwrite("1", sizeof(char), 1, Handle);
			fclose(Handle);
		}
	}
	int on( int duration ) {
		FILE *Handle = NULL;
		if((Handle = fopen(Brightness, "r+")) != NULL){
			fwrite("1", sizeof(char), 1, Handle);
			fclose(Handle);
		}
		usleep(duration);
		Handle = NULL;
		if((Handle = fopen(Brightness, "r+")) != NULL){
			fwrite("0", sizeof(char), 1, Handle);
			fclose(Handle);
		}
	}
	int off() {
		FILE *Handle = NULL;
		if((Handle = fopen(Brightness, "r+")) != NULL){
			fwrite("0", sizeof(char), 1, Handle);
			fclose(Handle);
		}
	}
private:
	signed short int index;
	char* Brightness;
};

int ULedInit() {
	FILE* Handle = NULL;
	if((Handle = fopen(LED[0], "r+")) != NULL){
		fwrite("0", sizeof(char), 1, Handle);
 		fclose(Handle);
	}
	if((Handle = fopen(LED[1], "r+")) != NULL){
		fwrite("0", sizeof(char), 1, Handle);
 		fclose(Handle);
	}
	if((Handle = fopen(LED[2], "r+")) != NULL){
		fwrite("0", sizeof(char), 1, Handle);
 		fclose(Handle);
	}
	if((Handle = fopen(LED[3], "r+")) != NULL){
		fwrite("0", sizeof(char), 1, Handle);
 		fclose(Handle);
	}
	return 0;
}

int blink( int bps, int time ) {
	UserLed ULED[] = { 0, 1, 2, 3 };
	int intervall = 1000000 / bps;
	int times = (time * bps)/4;
	for (int t = 0; t < times; t++) {
		for (int L = 0; L < 4; L++) {
			ULED[L].on(intervall);
		}
	}
}
	
