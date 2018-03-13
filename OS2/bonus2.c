#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

const int LedArray[4] = {LED1, LED2, LED3, LED4};

int sudoku[9][9];

typedef struct {
    int index;
    int (*sudoku)[9][9];
} parameters;


void *check_row(void * args){
    parameters* params = (parameters *) args;
    //printf("check_row thread created %d\n", params->index);

    //int result=0;
    int error_flag=0;

    for(int i=1; i<10; i++){
    		for(int j=0; j<9; j++){
    			if (sudoku[params->index][j] == i){
    				error_flag++;
    				break;
    			}
    		}
    }
    //result += sudoku[params->index][i];
    //printf("result row %d: %d\n", params->index, result);
    if (error_flag == 9){
    		return EXIT_SUCCESS;
    }
    else{
    		return EXIT_FAILURE;
    }
}

void *check_colum(void * args){
	parameters* params = (parameters *) args;
    //printf("check_colum thread created\n");

    int result=0;

    for(int i=0; i<9; i++){
    		result += sudoku[i][params->index];
    }

    int error_flag=0;

	for(int i=1; i<10; i++){
		for(int j=0; j<9; j++){
			if (sudoku[j][params->index] == i){
				error_flag++;
				break;
			}
		}
	}


    //printf("result col %d: %d\n", params->index, result);

	if (error_flag==9){
			return EXIT_SUCCESS;
	}
	else{
			return EXIT_FAILURE;
	}
}


void *check_sub(void * args){
	parameters* params = (parameters *) args;
    //printf("check_sub thread created\n");

    int cubicle = params->index;
    //int error_flag=FALSE;

    int xpos = 0;
    	int ypos = 0;

    if(cubicle >= 0 && cubicle < 3){
    		xpos = cubicle*3;
    		ypos = 0;
    }
    if(cubicle >= 3 && cubicle < 6){
    		xpos = (cubicle-3)*3;
    		ypos = 3;
    }
    if(cubicle >= 6 && cubicle < 9){
		xpos = (cubicle-6)*3;
		ypos = 6;
	}
    //printf("index: %d, xpos: %d, ypos: %d\n", cubicle, xpos, ypos);

    int error_flag=0;


    for(int k=1; k<10; k++){
		for(int i=ypos; i<ypos+3; i++){
			for(int j = xpos; j<xpos+3; j++){
				if(sudoku[i][j] == k){
					error_flag++;
					break;
				}
			}
    }
    }
    //printf("result sub %d: %d\n", cubicle, result);

    if (error_flag == 9){
    		return EXIT_SUCCESS;
    }
    else{
    		return EXIT_FAILURE;
    }
}



int main( int argc, char *argv[] ) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    //int sudoku[9][9];
    pthread_t row_threads[9];
    pthread_t colum_threads[9];
    pthread_t sub_threads[9];
    //void * exitCodes[9][9];

    fp = fopen("./sudoku.txt", "r");
    if (fp == NULL) {
        printf("Error reading file\n");
    }


    int row = 0;
    int colum = 0;


    while ((read = getline(&line, &len, fp)) != -1) {
        for (int j = 0; j < strlen(line); j++){
            if(isdigit(line[j])){
                sudoku[row][colum]= ((int)line[j] - 48);
                //printf("[%d][%d]: %d \n",row, colum, sudoku[row][colum]);
                colum++;

            }
        }
        row++;
        colum=0;
    }



    for(int i = 0; i<9; i++){

        //static parameters args;
        //static parameters* args_ptr = &args;

        parameters *params = (parameters *) malloc(sizeof(parameters));
        params->sudoku = &sudoku;
        params->index = i;

        pthread_create(&row_threads[i], NULL, check_row, params);
        pthread_create(&colum_threads[i], NULL, check_colum, params);
        pthread_create(&sub_threads[i], NULL, check_sub, params);
    }

    void *status;

    int error_flag = FALSE;

    for(int i=0; i<9; i++){

    		pthread_join(row_threads[i], &status);
    		if( (int) status ){
    			error_flag = TRUE;
    			printf("error in row %d\n", i);
    		}

    		pthread_join(colum_threads[i], &status);
    		if( (int) status ){
    			error_flag = TRUE;
    			printf("error in column %d\n", i);
		}

    		pthread_join(sub_threads[i], &status);
    		if( (int) status ){
    			error_flag = TRUE;
    			printf("error in sub %d\n", i);
    		}
    }

    if(!error_flag){
    		printf("Success!!\n");
    }

    return EXIT_SUCCESS;
}






