#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define SIZE 1000
#define NUM_LINES 50

char *buffer_1[NUM_LINES];
// char buffer_2[SIZE];
// number of items in buffer 1
int count_1 = 0;
// index of the producer/input thread is going to be
int prod_idx_1 = 0;
// index of the consumer/output thread is going to be
int con_idx_1 = 0;

// void put_buff_2(){
//     for(int i=0;i<strlen(buffer_1);i++){
//         buffer_1[prod_idx_1] = line[i];
//         prod_idx_1 +=1;
//         count_1++;
//     }
// }
char* get_buff_1(){
    char* aStringInput = buffer_1[con_idx_1];
    count_1--;
    return aStringInput;
}
void line_seperator(){
    char* aString = get_buff_1();
    for(int i=0;i<       ;i++){
        if(buffer_1[i] == '\n'){
            buffer_1[i] = ' ';
        }
    }
    put_buff_2();
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
    for (int i=0; i<NUM_LINES; i++){
        char *line_input = get_user_input();
        // fgets(line_input, SIZE, stdin);
        put_buff_1(line_input);
    }
}
int main(int argc, char *argv[]){

    get_input();
    line_seperator();
    return 0;
}

