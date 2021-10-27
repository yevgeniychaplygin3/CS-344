#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_CHARS 2048 
#define MAX_ARGS 512
struct command{
    char user_command[MAX_CHARS];
    char user_args[MAX_ARGS];
    char input_redir[255];
    char output_redir[255];

};
void changeDir(char* userInput){
    // check to see if it has args
    int i=1;
    // if it does not have args then chdir to HOME 

    // if it has args then change to absolute or relative path


}
void check_builtIn(char* userInput){
    // printf("%s", userInput);
    if(strcmp(userInput, "cd") == 0){
        printf("userinput == cd\n");
        changeDir(userInput);
    }
    else if(strcmp(userInput, "exit") == 0){
        printf("userinput == exit\n");
    }
    else if(strcmp(userInput, "status") == 0){
        printf("status== exit\n");
    }
    else{
        printf("not a built-in command\n");
    }
}

// STOPPED HERE AT TRYING TO FIND A WAY TO HOLD THE USER DATA BY PARSING..
// MAYBE USE **
struct command *parseInput(char *input){
    struct command *currCommand = malloc(sizeof(struct command));
    // struct command currCommand = malloc(sizeof(struct command));

    char *saveptr;
    char **args[MAX_ARGS];
    memset(args, '\0', MAX_ARGS);
    
    
    // command
    char *token = strtok_r(input, " ", &saveptr);
    // currCommand.user_command[] = calloc(strlen(token), sizeof(char));
    strcpy(currCommand->user_command, token);
    
    // user_args
    token = strtok_r(NULL, " ", &saveptr);
    // currCommand.user_args = calloc(strlen(token), sizeof(char));
    strcpy(currCommand->user_args, token);


    // input_redir
    token = strtok_r(NULL, " ", &saveptr);
    // currCommand.input_redir = calloc(strlen(token), sizeof(char));
    strcpy(currCommand->input_redir, token);

    // output_redir
    token = strtok_r(NULL, " ", &saveptr);
    // currCommand.output_redir = calloc(strlen(token), sizeof(char));
    strcpy(currCommand->output_redir, token);
    
    return currCommand;
}

int main()
{
    while(1){
        int numArgs;
        
        char userInput[MAX_CHARS];
        struct command *currCommand = malloc(sizeof(struct command));
        
        // display prompt
        printf(": ");
        fgets(userInput, MAX_CHARS, stdin);
        
        // get rid of new line character??
        currCommand = parseInput(userInput);
        char *firstArg = currCommand->user_command;
        check_builtIn(firstArg);
    }
    
    return 0;
}
