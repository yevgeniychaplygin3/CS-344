/*Create a key of specified length using any 26 Uppercase letters and a space*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    if(argc < 2){printf("Usage: %s keylength\n", argv[0]); exit(1);}

    int n = atoi(argv[1]);
    char key[n];
    char space = 32;
    memset(key, space, n);

    time_t t;
    srand((unsigned) time(&t));
    for(int i=0; i<n; i++){
        char letter = rand() %(90-65+1)+65;
        // printf("letter:%c\tnumber:%d\n",letter, letter);
        // if its even
        if( (rand() / 3) % 13  == 0){
            int randomIndex = rand() % n;
            
            key[randomIndex] = space;
        }
        else{
            key[i] = letter;
        }
        // printf("%c", key[i]);
    }
    write(1, key, sizeof(key));
    write(1, "\n", 1);
        // printf("\n");
    
    return 0;
}
