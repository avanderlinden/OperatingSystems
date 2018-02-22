/*
 ============================================================================
 Name        : os_1_1.c
 Author      : Alex van der Linden
 Version     :
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
	char student_name[] = "Alex van der Linden";
	char student_number[] = "4500806";


	printf("Student Name \t: %s\n", student_name);
	printf("Student Number \t: %s\n", student_number);

	printf("\nProcess ID : %d\n", (int) getpid());


	return EXIT_SUCCESS;
}
