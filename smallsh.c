#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_CHARS 2048 
#define MAX_ARGS 512
struct command{
    char user_command[255];
    char *user_args[MAX_ARGS];
    char input_redir[255];
    char output_redir[255];

};

// make sure to check to see that path exists or not
void changeDir(struct command *currCommand){
    // check to see if it has args
    int check = 1;
    char s[100];
    char *homePath;
    printf("%s\n", getcwd(s, 100));
    if(*currCommand->user_args == NULL){
        // printf("no args");
    }
    else{
        // printf("%s", *currCommand->user_args);
        check = 0;
    }
    switch (check){
        // change to specified path name 
    // if it has args then change to absolute or relative path
        case 0:
                chdir(*currCommand->user_args);
                printf("%s\n", getcwd(s, 100));
                break;
        // change to HOME
        // if it does not have args then chdir to HOME 
        case 1:
                homePath = getenv("HOME");
                chdir(homePath);
                printf("%s\n", getcwd(s, 100));
                break;
    }
}
void printStatus(){
    int status;
    if(WIFEXITED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Exit status of the child was %d\n", exit_status);
    }
    else if(WIFSIGNALED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Terminated by signal %d\n", WTERMSIG(exit_status));
        fflush(stdout);
    }
}

void check_builtIn(struct command *currCommand){

    char *firstArg = currCommand->user_command;

    if(strcmp(firstArg, "cd") == 0){
        printf("userinput == cd\n");
        changeDir(currCommand);
    }
    else if(strcmp(firstArg, "exit") == 0){
        printf("userinput == exit\n");
        exit(0);
    }
    else if(strcmp(firstArg, "status") == 0){
        printf("status== exit\n");
        printStatus();
    }
    else{
        printf("not a built-in command\n");
    }
}

//debuginfo-install glibc-2.17-325.el7_9.x86_64
int getArgNumber(char *input){
    int count;
    for(int i=0; input[i]; i++){
        if(input[i]== 32){              //if it's equal to ASCII white space or ASCII newline
            count++;        
        }
        // strip the new line character && try to strip white space from before newline
        if(input[i] == '\n'){                   
            input[i] = '\0';
            if(input[i-1] == 32){
                input[i-1] = '\0';
                count--;
            }
        }
        
    }
    return count;
}

char* varExpand(char *token){
    // copy string to new string
    int len = strlen(token);
    char expanVar[len];
    strcpy(expanVar, token);

    int varPid = getpid();

    for (int i=0; i<strlen(token);i++){
        // set up formating for sprintf. $$ should now be %d
        if(token[i] == '$' && token[i+1] == '$')
        {
            if(strlen(token) > i+1){
                expanVar[i] = '%';
                expanVar[i+1] = 'd';
            }
        }
        
    }

    sprintf(token, expanVar, varPid);
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
        // char *firstArg = currCommand->user_command;
        // if its not built in then execute exec()
        if(!check_builtIn(currCommand)){

        }
    }
    return 0;
}
