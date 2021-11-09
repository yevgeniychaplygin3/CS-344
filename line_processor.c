#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define SIZE 1000
#define NUM_LINES 50
//check for Stop-processing line
int check = 0;
char *buffer_1[NUM_LINES], *buffer_2[NUM_LINES], *buffer_3[NUM_LINES];
// number of items in buffer
int count_1 = 0, count_2 = 0, count_3 = 0;
// index of the producer/input thread is going to be
int prod_idx_1 = 0, prod_idx_2 = 0, prod_idx_3 = 0;
// index of the consumer/output thread is going to be
int con_idx_1 = 0, con_idx_2 = 0, con_idx_3 = 0;
// Initialize the mutex for buffer
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER, mutex_2 = PTHREAD_MUTEX_INITIALIZER, mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER, full_2 = PTHREAD_COND_INITIALIZER, full_3 = PTHREAD_COND_INITIALIZER;

char* get_buff_3(){
    char* aStringInput = buffer_3[con_idx_3];
    con_idx_3 = con_idx_3 + 1;
    count_3--;
    return aStringInput;
}
void* output_thread(void *args){
    // get only 80 chars & stop when theres a stop
    char printBuffer[80];
        memset(printBuffer, '\0', 80);
    // get the string at the index
    char* printString = get_buff_3();
    
    for(int i=0;i<2;i++){
        strncpy(printBuffer, printString, strlen(printString));
        if(printBuffer[79]!='\0'){
            printf("%s", printBuffer);
            memset(printBuffer, '\0', 80);
        }
        printString = get_buff_3();
    }

    // strcat(printBuffer, printString);
    // int len; 
    // len = strlen(printBuffer);
    // while(len < 80){
        // printString = get_buff_3();
        // if( (strlen(printBuffer) + strlen(printString) > 80){
            // need to keep track of extra characters. 
        // }
        // strncat(printBuffer, printString, 80-strlen(printString));
        // len = strlen(printBuffer);
    // }
    // check to see if all 80 chars are full.
    // if(printArray[80]!= '\0'){
    // }
    printf("Final output string: %s\n", printBuffer);
    
    return NULL;
}
void put_buff_3(char* string3){
    // add in mutex & condition variables
    buffer_3[prod_idx_3] = string3;
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
}
char* get_buff_2(){
    // add in mutex & condition variables
    char* aStringInput = buffer_2[con_idx_2];
    con_idx_2 = con_idx_2 + 1;
    count_2--;
    return aStringInput;
}

void* plus_sign_thread(void *args){
    for(int i=0; i<2;i++){
        char *spaceSeperatedString = get_buff_2();
        int len = strlen(spaceSeperatedString);
        char copyString[len];
        for (int j =0; j<len; j++){
            strcpy(copyString, spaceSeperatedString);
            if(copyString[j] == '+' && copyString[j+1] == '+' && j+1<len){
                copyString[j] = '%';
                copyString[j+1] = 'c';
                sprintf(spaceSeperatedString, copyString, '^');
            }
        }
        put_buff_3(spaceSeperatedString);
    }
    return NULL;
}
void put_buff_2(char *line_seperator_string){
    // add in mutex & condition variables
    buffer_2[prod_idx_2] = line_seperator_string;
    prod_idx_2 +=1;
    count_2++;
}
char* get_buff_1(){
    // add in mutex & condition variables
    char* aStringInput = buffer_1[con_idx_1];
    con_idx_1 = con_idx_1 + 1;
    count_1--;
    return aStringInput;
}
void *line_seperator(void *args){
    for(int i=0;i<2;i++){
        char* aString = get_buff_1();
        int newLineIndex = strlen(aString)-1;
        if(aString[newLineIndex] == '\n'){
            char *lastWord = &aString[newLineIndex];
            lastWord = lastWord-4;
            if(strcmp(lastWord, "STOP\n") == 0){
                // printf("stop processing now.");
                check = 1;
            }
            aString[newLineIndex] = ' ';
            put_buff_2(aString);
        }
    }
    return NULL;
}
void put_buff_1(char *line){
    // add in mutex & condition variables
    buffer_1[prod_idx_1] = line;
    prod_idx_1 +=1;
    count_1++;
}
char* get_user_input(){
    char *user_input = calloc(SIZE, sizeof(char));
    printf("Enter an input (no longer than 1000 characters): ");
    fgets(user_input, SIZE, stdin);
    return user_input;
}
void *get_input(void *args){
    for (int i=0; i<2; i++){
        if(check == 1){return NULL;}
        char *line_input = get_user_input();
        // fgets(line_input, SIZE, stdin);
        put_buff_1(line_input);
    }
    return NULL;
}
int main(int argc, char *argv[]){
    if(argc < 2){
        printf("no input files"); 
        get_input(NULL);
    }else{
        printf("file:%s argc#:%d", argv[1], argc);
        // FILE *fp = fopen(argv[1], "r");
    }
   
    line_seperator(NULL);
    plus_sign_thread(NULL);
    output_thread(NULL);
    // pthread_t input_t, line_seperator_t; //, plus_sign_t, output_t;
    // pthread_create(&input_t, NULL, get_input, NULL);
    // pthread_create(&line_seperator_t, NULL, line_seperator, NULL);
    // pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
    // pthread_create(&output_t, NULL, output_thread, NULL);
    // pthread_join(input_t, NULL);
    // pthread_join(line_seperator_t, NULL);
    // pthread_join(plus_sign_t, NULL);
    // pthread_join(output_t, NULL);
    return 0;
}
