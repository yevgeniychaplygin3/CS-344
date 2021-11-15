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
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER; 
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

char* get_buff_3(){
    printf("locking get buff3...\n");
    pthread_mutex_lock(&mutex_3);
    
    while(count_3 == 0){
        printf("waiting getbuff3...\n");
        pthread_cond_wait(&full_3, &mutex_3);
    }
    printf("get_buff_3() Done waiting \n");

    char* aStringInput = buffer_3[con_idx_3];
    con_idx_3 = con_idx_3 + 1;
    count_3--;
    printf("unlocking getbuff3...\n");
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
                printf("End of file processing\n");
                i += 100;
                break;
            }

            strcat(completeMessage, getString);

        }while(completeMessage[80] == '\0');


        // strcat(completeMessage, getString);
        // // keep concatenating until we have at least 80 in completeMessage
        // while(completeMessage[80] == '\0'){
        //     getString = get_buff_3();
        //     if(getString == NULL){return NULL;}

        // }
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
    // add in mutex & condition variables
    printf("locking get putbuff3...\n");
    pthread_mutex_lock(&mutex_3);
    buffer_3[prod_idx_3] = string3;
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
    printf("signaling full3...\n");
    pthread_cond_signal(&full_3);
    printf("unlocking putbuff3...\n");
    pthread_mutex_unlock(&mutex_3);
}
char* get_buff_2(){
    // add in mutex & condition variables
    printf("locking get getbuff2...\n");
    pthread_mutex_lock(&mutex_2);
    while(count_2==0){
        printf("waiting get buff2...\n");

        pthread_cond_wait(&full_2, &mutex_2);
    }
    printf("get_buff_2() Done waiting \n");

    char* aStringInput = buffer_2[con_idx_2];
    con_idx_2 = con_idx_2 + 1;
    count_2--;
    printf("unlocking get buff2...\n");
    pthread_mutex_unlock(&mutex_2);
    return aStringInput;
}

void* plus_sign_thread(void *args){
    for(int i=0; i<NUM_LINES;i++){
        char *spaceSeperatedString = get_buff_2();
        if(spaceSeperatedString == NULL){break;}
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
    printf("locking get putbuff2...\n");
    pthread_mutex_lock(&mutex_2);
    buffer_2[prod_idx_2] = line_seperator_string;
    prod_idx_2 +=1;
    count_2++;
    printf("signaling full2...\n");
    pthread_cond_signal(&full_2);
    printf("unlocking putbuff2...\n");
    pthread_mutex_unlock(&mutex_2);
}
char* get_buff_1(){
    // add in mutex & condition variables
    printf("locking get buff1...\n");
    pthread_mutex_lock(&mutex_1);
    while(count_1 == 0){
        // if(strcmp(aStringInput, "-1") == 0){
        //     return NULL;
        // }
        printf("waiting get buff1...\n");
        pthread_cond_wait(&full_1, &mutex_1);
    }
    printf("get_buff_1() Done waiting \n");
    // aStringInput = buffer_1[con_idx_1];
    char* aStringInput = buffer_1[con_idx_1];

    con_idx_1 = con_idx_1 + 1;
    count_1--;
    printf("unlocking get buff1...\n");
    pthread_mutex_unlock(&mutex_1);
    return aStringInput;
}
void *line_seperator(void *args){
    for(int i=0;i<NUM_LINES;i++){
        char* aString = get_buff_1();
        if(aString == NULL){break;}
        int newLineIndex = strlen(aString)-1;
        if(aString[newLineIndex] == '\n'){
            if(aString[newLineIndex-1] == ' '){
                aString[newLineIndex] = '\0';
            }else{
                aString[newLineIndex] = ' ';
            }
        }
        put_buff_2(aString);
    }
    return NULL;
}
void put_buff_1(char *line){
    // add in mutex & condition variables
    printf("locking buff1...\n");
    pthread_mutex_lock(&mutex_1);
    buffer_1[prod_idx_1] = line;
    prod_idx_1 = prod_idx_1 + 1;
    count_1++;
    printf("signaling full1...\n");
    pthread_cond_signal(&full_1);
    printf("unlocking buff1...\n");
    pthread_mutex_unlock(&mutex_1);
}
char* get_user_input(void *args){
    char *user_input = calloc(SIZE, sizeof(char));
    // printf("Enter an input (no longer than 1000 characters): ");
    fgets(user_input, SIZE, stdin);
    return user_input;
}
void *get_input(void *args){
    for (int i=0; i<NUM_LINES; i++){
        char *line_input = get_user_input(args);
        char* ret = strstr(line_input, "STOP\n");
        if(ret != NULL)
        {
            if(strcmp(ret-1, " STOP\n") == 0 || strcmp(ret-1, "") == 0){
                *ret = '@';
                ret[1] = '@';
                ret[2] = '\n';
                ret[3] = '\0';
                put_buff_1(line_input);
                break;
            }
        }
        put_buff_1(line_input);
    }
    put_buff_1(specialmarker);
    return NULL;
}
int main(int argc, char *argv[]){
    // get_input(NULL); line_seperator(NULL); plus_sign_thread(NULL); output_thread(NULL);
    pthread_t input_t, line_seperator_t, plus_sign_t, output_t;
    // create threads
    pthread_create(&input_t, NULL, get_input, NULL);
    pthread_create(&line_seperator_t, NULL, line_seperator, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
    pthread_create(&output_t, NULL, output_thread, NULL);
    // wait for threads to terminate
    
    int retVal1, retVal2,retVal3,retVal4;
    retVal1 = pthread_join(input_t, NULL);
    printf("\n\n\nt1..%d\n", retVal1);
    retVal2 = pthread_join(line_seperator_t, NULL);
    printf("\n\n\nt2..%d\n", retVal2);
    retVal3 = pthread_join(plus_sign_t, NULL);
    printf("\n\n\nt3..%d\n", retVal3);
    retVal4 = pthread_join(output_t, NULL);
    printf("\n\n\nt4..%d\n", retVal4);

    return 0;
}
