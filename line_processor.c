#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define SIZE 1000
#define NUM_LINES 50

char *buffer_1[NUM_LINES], *buffer_2[NUM_LINES], *buffer_3[NUM_LINES];
// number of items in buffer 1
int count_1 = 0, count_2 = 0, count_3 = 0;
// index of the producer/input thread is going to be
int prod_idx_1 = 0, prod_idx_2 = 0, prod_idx_3 = 0;
// index of the consumer/output thread is going to be
int con_idx_1 = 0, con_idx_2 = 0, con_idx_3 = 0;

char* get_buff_3(){
    char* aStringInput = buffer_3[con_idx_3];
    con_idx_3 = con_idx_3 + 1;
    count_2--;
    return aStringInput;
}
void output_thread(){
    char* printString = get_buff_3();
    printf("Final output string: %s\n", printString);
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

void plus_sign_thread(){
    for(int i=0; i<2;i++){
        char *spaceSeperatedString = get_buff_2();
        int len = strlen(spaceSeperatedString);
        char copyString[len];
        strcpy(copyString, spaceSeperatedString);
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
void line_seperator(){
    for(int i=0;i<2;i++){
        char* aString = get_buff_1();
        int newLineIndex = strlen(aString)-1;
        if(aString[newLineIndex] == '\n'){
            aString[newLineIndex] = ' ';
            printf("new line\n");
            put_buff_2(aString);
        }
    }
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
void get_input(){
    for (int i=0; i<2; i++){
        char *line_input = get_user_input();
        // fgets(line_input, SIZE, stdin);
        put_buff_1(line_input);
    }
}
int main(int argc, char *argv[]){
    get_input();
    line_seperator();
    plus_sign_thread();
    output_thread();
    return 0;
}
