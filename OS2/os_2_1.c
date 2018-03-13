#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

const int LedArray[4] = {LED1, LED2, LED3, LED4};

void led_it_shine(int value){

	int ArrayLength = (int) (sizeof(LedArray) / sizeof(LedArray[0])) -1;

	for(int i = ArrayLength; i>=0; i--) {
		int mask = 0;
		mask = 1 << i;

		if(mask & value) {
			digitalWrite(LedArray[i], HIGH);
			printf("1");
		}
		else {
			digitalWrite(LedArray[i], LOW);
			printf("0");
		}
	}
	printf("\n");
}

int main( int argc, char *argv[] ) {

	wiringPiSetup();

	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);


	digitalWrite(LED1, HIGH);
	digitalWrite(LED1, LOW);

	int value = 0;
	char *endptr;

	if( argc == 2 ) {
	      value = (int) strtol(argv[1], &endptr, 10);
	      if (value < 0 || value > 15 || endptr == argv[1]){
	    	  	  printf("Illigal input value, expecting decimal number "
	    	  	    	  			"between 0-15\n");
	      }
	      else {
	    	  	  led_it_shine(value);
	      }
	   }
	   else if( argc > 2 ) {
	      printf("Too many arguments, expecting 1 "
	    		  "decimal number between 0-15.\n");
	   }
	   else {
	      printf("Too few arguments, expecting 1 "
	    		  "decimal number between 0-15.\n");
	   }

	return EXIT_SUCCESS;
}

