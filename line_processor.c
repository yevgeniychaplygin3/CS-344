#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define SIZE 1000
#define NUM_LINES 50
char *specialmarker = "-1";
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
    pthread_mutex_lock(&mutex_3);
    while(count_3 == 0){
        pthread_cond_wait(&full_3, &mutex_3);
    }
    char* aStringInput = buffer_3[con_idx_3];
    con_idx_3 = con_idx_3 + 1;
    count_3--;
    pthread_mutex_unlock(&mutex_3);
    return aStringInput;
}
void* output_thread(void *args){
    char readBuffer[81];    //read buffer to hold 80 chars
    char completeMessage[1000];
    memset(readBuffer, '\0', 81);
    memset(completeMessage, '\0', 1000);
    // get the string at the index
    char* getString;
    for(int i=0;i<NUM_LINES;i++){
        // keep concatenating until we have at least 80 in completeMessage
        do{
            getString = get_buff_3();
            if(getString == NULL){break;}
            
            if(strstr(getString, "@@") != NULL){
                // printf("End of file processing\n");
                i += 100;
                char *endIndex = strstr(getString, "@@");
                *endIndex = '\0';

                strcat(completeMessage, getString);

                break;
            }

            strcat(completeMessage, getString);

        }while(completeMessage[80] == '\0');

        // print the first 80. 
        strncpy(readBuffer, completeMessage, 80);
        readBuffer[80] = '\n';
        if(strlen(readBuffer) >= 80){
            write(1, readBuffer, 81);
        }
        memmove(completeMessage, completeMessage+80, sizeof(completeMessage) - sizeof(*completeMessage));
    }
    return NULL;
}
void put_buff_3(char* string3){
    pthread_mutex_lock(&mutex_3);
    buffer_3[prod_idx_3] = string3;
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
    pthread_cond_signal(&full_3);
    pthread_mutex_unlock(&mutex_3);
}
char* get_buff_2(){
    pthread_mutex_lock(&mutex_2);
    while(count_2==0){
        pthread_cond_wait(&full_2, &mutex_2);
    }
    char* aStringInput = buffer_2[con_idx_2];
    con_idx_2 = con_idx_2 + 1;
    count_2--;
    pthread_mutex_unlock(&mutex_2);
    return aStringInput;
}

void* plus_sign_thread(void *args){
    // for(int i=0; i<NUM_LINES;i++){
    char *spaceSeperatedString = "";
    while (strcmp(spaceSeperatedString, specialmarker) != 0){
        spaceSeperatedString = get_buff_2();

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
    // put_buff_3(specialmarker); //might not need because we already put it in the first one.
    return NULL;
}
void put_buff_2(char *line_seperator_string){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
    buffer_2[prod_idx_2] = line_seperator_string;
    // Increment the index where the next item will be put.
    prod_idx_2 = prod_idx_2 + 1;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
}
char* get_buff_1(){
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    while(count_1 == 0){
        // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    }
    char* aStringInput = buffer_1[con_idx_1];
    // Increment the index from which the item will be picked up
    con_idx_1 = con_idx_1 + 1;
    count_1--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
    return aStringInput;    // Return the line
}
void *line_seperator(void *args){
    char *aString = "";

    while(strcmp(aString, specialmarker) != 0){

        aString = get_buff_1();

        int newLineIndex = strlen(aString)-1;
        if(aString[newLineIndex] == '\n'){
            if(aString[newLineIndex-1] == ' '){
                aString[newLineIndex] = '\0';
            }else{
                aString[newLineIndex] = ' ';
            }
            put_buff_2(aString);
        }
    }
    put_buff_2(specialmarker);
    return NULL;
}
void put_buff_1(char *line){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    buffer_1[prod_idx_1] = line;
    // Increment the index where the next item will be put.
    prod_idx_1 +=1;
    count_1++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}

void *get_input(void *args){
    for (int i=0; i<NUM_LINES; i++){
        // get inpuut from stdin
        char *line_input = calloc(SIZE, sizeof(char));
        fgets(line_input, SIZE, stdin);
        
        // check to see if it has is a stop processing line
        char* ret = strstr(line_input, "STOP\n");
        if(ret != NULL)
        {
            // check to see if it has a prefix or sufix 
            if(strcmp(ret-1, " STOP\n") == 0 || strcmp(ret-1, "") == 0){
                *ret = '@';
                ret[1] = '@';
                ret[2] = '\n';
                ret[3] = '\0';
                put_buff_1(line_input);
                break;  // end of processing. Break out of the loop
            }
        }
        put_buff_1(line_input);
    }
    put_buff_1(specialmarker); // put -1 into the buffer to let the next thread know there won't be anymore input. 

    return NULL;
}
int main(int argc, char *argv[]){
    // get_input(NULL); line_seperator(NULL); plus_sign_thread(NULL); output_thread(NULL);
    pthread_t input_t, line_seperator_t, plus_sign_t, output_t;
    // // create threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_seperator_t, NULL, line_seperator, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
    pthread_create(&output_t, NULL, output_thread, NULL);
    // // wait for threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_seperator_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);

    // int retVal1, retVal2,retVal3,retVal4;
    // retVal1 = pthread_join(input_t, NULL);
    // printf("\n\n\nt1..%d\n", retVal1);
    // retVal2 = pthread_join(line_seperator_t, NULL);
    // printf("\n\n\nt2..%d\n", retVal2);
    // retVal3 = pthread_join(plus_sign_t, NULL);
    // printf("\n\n\nt3..%d\n", retVal3);
    // retVal4 = pthread_join(output_t, NULL);
    // printf("\n\n\nt4..%d\n", retVal4);
    return 0;
}

