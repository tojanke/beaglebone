FILE *LEDHandle = NULL;
char *LED[] = {	"/sys/class/leds/beaglebone:green:usr0/brightness", 
		"/sys/class/leds/beaglebone:green:usr1/brightness", 
		"/sys/class/leds/beaglebone:green:usr2/brightness", 
		"/sys/class/leds/beaglebone:green:usr3/brightness"};

int turnon (int index){
	if((LEDHandle = fopen(LED[index - 1], "r+")) != NULL){
      		fwrite("1", sizeof(char), 1, LEDHandle);
      		fclose(LEDHandle);
		return 0;
	}
	else {
		return 1;
    	}
};

int turnoff (int index){
	if((LEDHandle = fopen(LED[index - 1], "r+")) != NULL){
      		fwrite("0", sizeof(char), 1, LEDHandle);
      		fclose(LEDHandle);
		return 0;
	}
	else {
		return 1;
    	}
};
