#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_CHARS 2048 
#define MAX_ARGS 512

static int status;

struct command{
    char user_command[255];
    char *user_args[MAX_ARGS];
    char input_redir[255];
    char output_redir[255];
    int background;
};


// make sure to check to see that path exists or not
void changeDir(struct command *currCommand){
    // check to see if it has args
    int check = 1;
    char s[100];
    char *homePath;
    printf("%s\n", getcwd(s, 100));
    if(*currCommand->user_args == NULL){
        return;
    }
    else{
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


void printStatus(int status){
    if(WIFEXITED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Exit status was %d\n", exit_status);
    }
    else if(WIFSIGNALED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Terminated by signal %d\n", WTERMSIG(exit_status));
        fflush(stdout);
    }
}


int check_builtIn(struct command *currCommand){

    char *firstArg = currCommand->user_command;

    if(strcmp(firstArg, "cd") == 0){
        changeDir(currCommand);
        return 1;
    }
    else if(strcmp(firstArg, "exit") == 0){
        exit(0);
    }
    else if(strcmp(firstArg, "status") == 0){
        // static int status;
        printStatus(status);
        return 1;
    }
    else{
        return 0;
    }
}


int getArgNumber(char *input){
    int count;
    for(int i=0; input[i]; i++){ 
        // if it's equal to ASCII white space or ASCII newline
        if(input[i]== 32){             
            count++;        
        }
        // strip the new line character & white space from before newline
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
                break;
            }
        }
        
    }

    sprintf(token, expanVar, varPid);
    return token;
}


struct command *parseInput(char *input){
    struct command *currCommand = malloc(sizeof(struct command));

    int argNum = getArgNumber(input);

    char *saveptr;

    char *token = strtok_r(input, " ", &saveptr);;
    strcpy(currCommand->user_command, token);

    // make the first command also the argument for the array
    currCommand->user_args[0] = currCommand->user_command;

    // for each argument tokenize and put that argument into struct
    for(int i=1; i <= argNum; i++){     

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
        // check for background process. Check to see that it's the last character (i==argNum).
        else if(i == argNum && strstr(token, "&") != NULL)
        {
            // do something
            printf("Valid background request");
            currCommand->background = 1;
            currCommand->user_args[i] = token;
        }
        
        // then it is just an arg. 
        else{

            if(strstr(token, "$$") != NULL)
            {
                // do variable expansion
                token = varExpand(token);
            }
            
            char *argVar = currCommand->user_args[i];
            // currCommand->user_args[i] = calloc(strlen(token) + 1, sizeof(char)); 
            argVar = calloc(strlen(token) + 1, sizeof(char)); 
            
            strcpy(argVar, token);
            currCommand->user_args[i] = argVar;
        }
    }
        
    return currCommand;
}

void handleRedir(struct command *currCommand){
    char *file = NULL;

    if (currCommand->input_redir[0] != '\0')
    {
        file = currCommand->input_redir;
        int fd = open(file, O_RDONLY);
        if(fd<0){printf("Error opening the in file\n");exit(1);}

        dup2(fd, 0); 
    }
    if (currCommand->output_redir[0] != '\0'){
        file = currCommand->output_redir;
        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);

        if(fd<0){printf("Error opening the out file\n"); exit(1);}

        dup2(fd, 1);
    }
}

void backgroundRedir(struct command *currCommand){
    if (currCommand->input_redir[0] != '\0')
    {
        strcpy(currCommand->input_redir, "/dev/null");
        // currCommand->input_redir = "/dev/null";
    }
    if (currCommand->output_redir[0] != '\0'){
        strcpy(currCommand->output_redir, "/dev/null");
        // currCommand->output_redir = "/dev/null";
    }
}

void goFork(struct command *currCommand){
    pid_t spawnpid = -5;
    int childExitMethod = -5;

    spawnpid = fork();
    switch (spawnpid){
        case -1:
            perror("Error Forking");
            exit(1);
            break;
        // child fork
        case 0:
            if(currCommand->background == 1){
                backgroundRedir(currCommand);

            }
            handleRedir(currCommand);
           
            execvp(currCommand->user_command, currCommand->user_args);
            printf("ERROR, Command not found\n");
            exit(1);
        break;

        // parent fork
        default:
        if(currCommand->background == 1){
            printf("background pid is %d\n", getpid());
        }
        else{
            waitpid(spawnpid, &childExitMethod, 0);
            // check status
            status = childExitMethod;

        }
    }
}

int main()
{
    while(1){
        char userInput[MAX_CHARS];
        struct command *currCommand = malloc(sizeof(struct command));
        
        // display prompt
        printf(": ");
        fgets(userInput, MAX_CHARS, stdin);

        // check comments or blank lines
        if(userInput[0] == '#' || userInput[0] == '\n'){
            // printf("Comment or blank line entered");
            continue;
        }
        
        currCommand->background = 0;
        currCommand = parseInput(userInput);

        if(!check_builtIn(currCommand)){
            goFork(currCommand);

        }
    }
    return 0;
}
