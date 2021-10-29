#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_CHARS 2048 
#define MAX_ARGS 512
struct command{
    char user_command[255];
    char *user_args[MAX_ARGS];
    char input_redir[255];
    char output_redir[255];

};
// void changeDir(char* userInput){
//     // check to see if it has args
//     int i=1;
//     // if it does not have args then chdir to HOME 

//     // if it has args then change to absolute or relative path


// }
// void check_builtIn(char* userInput){
//     // printf("%s", userInput);
//     if(strcmp(userInput, "cd") == 0){
//         printf("userinput == cd\n");
//         changeDir(userInput);
//     }
//     else if(strcmp(userInput, "exit") == 0){
//         printf("userinput == exit\n");
//     }
//     else if(strcmp(userInput, "status") == 0){
//         printf("status== exit\n");
//     }
//     else{
//         printf("not a built-in command\n");
//     }
// }

//debuginfo-install glibc-2.17-325.el7_9.x86_64
int getArgNumber(char *input){
    int count;
    for(int i=0; input[i]; i++){
        if(input[i]== 32){ // || input[i]== 10){ //if it's equal to ASCII white space or ASCII newline
            count++;        
        }
        // strip the new line character
        if(strstr(input, "\n") != NULL){
            int len = strlen(input);
            char dest[len];
            strncpy(dest, input, len-1);
            strcpy(input, dest);
        }
    }
    return count;
}

//stopped at variable expansion.
// use for loop maybe and 
char* varExpand(char *token){

    printf("var expansion%s\n", token);
    
    sprintf(token, token, getpid())
    return token;
}
struct command *parseInput(char *input){
    struct command *currCommand = malloc(sizeof(struct command));
    // struct command currCommand = malloc(sizeof(struct command));
    int argNum = getArgNumber(input);

    // for each argument tokenize and put that argument into struct
    char *saveptr;

    char *token = strtok_r(input, " ", &saveptr);;
    strcpy(currCommand->user_command, token);

    for(int i=0; i < argNum; i++){              
        token = strtok_r(NULL, " ", &saveptr);
        // check for redirection
        if(strstr(token, "<") != NULL || strstr(token, ">") != NULL){
            if(strstr(token, "<") != NULL){
                strcpy(currCommand->input_redir, token+1);
            }
            if(strstr(token, ">") != NULL){
                strcpy(currCommand->output_redir, token+1);
            }
            
        }
        // check for background process
        else if(strstr(token, "&") != NULL)
        {
            // do something
        }
        else if(strstr(token, "$$") != NULL)
        {
            // do variable expansion
            token = varExpand(token);
        }
        // then it is just an arg. 
        else{
        char *argVar = currCommand->user_args[i];
        // currCommand->user_args[i] = calloc(strlen(token) + 1, sizeof(char)); 
        argVar = calloc(strlen(token) + 1, sizeof(char)); 

        
        strcpy(argVar, token);
        printf("hello4\n");
        currCommand->user_args[i] = argVar;
        }
        
    }
        
    return currCommand;
}

int main()
{
    while(1){
        // int numArgs;
        
        char userInput[MAX_CHARS];
        struct command *currCommand = malloc(sizeof(struct command));
        
        // display prompt
        printf(": ");
        fgets(userInput, MAX_CHARS, stdin);

        if(userInput[0] == '#' || userInput[0] == '\n'){
            printf("Comment or blank line entered");
            continue;
        }
        currCommand = parseInput(userInput);
        char *firstArg = currCommand->user_command;
        
        printf("here\n");
        // check_builtIn(firstArg);
    }
    
    return 0;
}
