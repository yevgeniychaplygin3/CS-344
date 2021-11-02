#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CHARS 2048 
#define MAX_ARGS 512

static int status;  // allow all functions to access the status
int ignore_background = 1;  // used to check if script should ignore background processes (0) or not (1)

// hold the users input
struct command{
    char user_command[255];
    char *user_args[MAX_ARGS];
    char input_redir[255];
    char output_redir[255];
    int background;
};


// change the directory to path specified by user, or HOME if there is no path
void changeDir(struct command *currCommand){
    // check used to see if it has args
    int check = 1;
    char *homePath;
    // char s[100];
    // printf("%s\n", getcwd(s, 100));

    // change to 0 if cd has any path arguments
    if(currCommand->user_args[1] != NULL){
       check = 0;
    }
    switch (check){
            // if it has args then change to specified path name. 
        case 0:
            chdir(currCommand->user_args[1]);
            // printf("%s\n", getcwd(s, 100));
            break;
        // if it does not have args then chdir to HOME 
        case 1:
            homePath = getenv("HOME");
            chdir(homePath);
            // printf("%s\n", getcwd(s, 100));
            break;
    }
}

// print the status of the previous command. Status is 0 if its first command ran, and ignores built in commands
void printStatus(int status){
    // if it exited successfully 
    if(WIFEXITED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Exit status was %d\n", exit_status);
        fflush(stdout);
    }
    // if it was killed by signal
    else if(WIFSIGNALED(status)){
        int exit_status = WEXITSTATUS(status);       
        printf("Terminated by signal %d\n", WTERMSIG(exit_status));
        fflush(stdout);
    }
}

// check to see if it matches the built in commands "status" "cd" "exit". Return 1 if its built in, and 0 if its not
int check_builtIn(struct command *currCommand){

    char *firstArg = currCommand->user_command;

    if(strcmp(firstArg, "cd") == 0){
        changeDir(currCommand);
        return 1;
    }
    else if(strcmp(firstArg, "exit") == 0){
        exit(0);    // exit shell program
    }
    else if(strcmp(firstArg, "status") == 0){
        printStatus(status);
        return 1;
    }
    else{
        return 0;
    }
}

// Get the number of arguments user passed in. Return that number. 
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
                count--; // then it is not an argument and subtract 1 from the count.
            }
        }
    }
    return count;
}

// expand the $$ in the command to be the pid of the parent shell.
char* varExpand(char *token){
    // copy string to expanVar
    int len = strlen(token);
    char expanVar[len];
    strcpy(expanVar, token);

    int varPid = getpid(); // parent pid
    
    // go through each char in the command that has $$ 
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
    // set token to be the new string with pid instead of $$ and return token.  
    sprintf(token, expanVar, varPid);
    return token;
}

// parse the users input. Place each input into the struct command.
struct command *parseInput(char *input){
    
    int argsDone = 0;   // used to check that all the args are parsed

    struct command *currCommand = malloc(sizeof(struct command));

    int argNum = getArgNumber(input); // get the number of arguments

    // start parsing the users input using strtok_r.
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
            // handle any white space after > or <
            // handle input
            if(strstr(token, "<") != NULL){

                // there is no space after "<"
                if(strcmp(token+1, "") != 0){
                    strcpy(currCommand->input_redir, token+1);
                }
                // there is a space after "<"
                else{
                    token = strtok_r(NULL, " ", &saveptr);
                    strcpy(currCommand->input_redir, token);
                    i++; // used so it is not counted as another argument
                }

            }

            // handle output
            if(strstr(token, ">") != NULL){
                // there is no space after ">"
                if(strcmp(token+1, "") != 0){
                strcpy(currCommand->output_redir, token+1);
                }
                // there is a space after ">"
                else{
                    token = strtok_r(NULL, " ", &saveptr);
                    strcpy(currCommand->output_redir, token);
                    i++; // used so it is not counted as another argument
                }
            }
            argsDone = 1;
        }
        // check for background process. Check to see that it's the last character (i==argNum).
        else if(i == argNum && strstr(token, "&") != NULL)
        {
            currCommand->background = 1;
        }

        
        // then it is just an argument if argDone is not 1. 
        else if(argsDone != 1){

            if(strstr(token, "$$") != NULL)
            {
                // do variable expansion
                token = varExpand(token);
            }
            
            char *argVar = currCommand->user_args[i];
            argVar = calloc(strlen(token) + 1, sizeof(char)); 
            
            strcpy(argVar, token);
            currCommand->user_args[i] = argVar;
        }
    }
        
    return currCommand;
}

// set the stdin and stdout to be the users command (input_redir/output_redir)
void handleRedir(struct command *currCommand){
    char *file = NULL;

    if (currCommand->input_redir[0] != '\0'){   // check to see if there is any user redirections
        file = currCommand->input_redir;
        int fd = open(file, O_RDONLY);
        if(fd<0){printf("Error opening the in file\n");exit(1);}

        dup2(fd, 0); // let stdin be the input_redir
    }
    if (currCommand->output_redir[0] != '\0'){  // check to see if there is any user redirections
        file = currCommand->output_redir;
        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0777);

        if(fd<0){printf("Error opening the out file\n"); exit(1);}

        dup2(fd, 1);    // let stdout be the output_redir
    }
}

// handle background command to have stdin and stdout go to /dev/null 
void backgroundRedir(struct command *currCommand){
    if (currCommand->input_redir[0] != '\0')
    {
        strcpy(currCommand->input_redir, "/dev/null");
    }
    if (currCommand->output_redir[0] != '\0'){
        strcpy(currCommand->output_redir, "/dev/null");
    }
}

// handle the ctrl-Z/SIGTSTP to switch between allowing background processes and not.
void handle_SIGTSTP(int signo){
    char* message;
    fflush(stdout);
    if(ignore_background == 1){
        message = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 50); // write is reentrant
        ignore_background = 0;
    }
    else{
        message = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 30);
        ignore_background = 1;
    }

}

// start forking off a new process, and initialize the signal handlers
void goFork(struct command *currCommand){
    // signal handlers
    // Initialize SIGINT_action struct to be empty
    struct sigaction SIGINT_action = {{0}};
    struct sigaction SIGTSTP_action = {{0}};

    // Fill out the SIGINT_action and SIGTSTP_action structs

    SIGINT_action.sa_handler = SIG_IGN; // set the ctrl-C/SIGINT to ignore it.
    SIGTSTP_action.sa_flags = SA_RESTART;

	// Register handle_SIGTSTP as the signal handler
    SIGTSTP_action.sa_handler = handle_SIGTSTP;

    // Install our signal handler
    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);


    pid_t spawnpid = -5;
    int childExitMethod = -5;
    // check to see if we should ignore background processes or not
    if(ignore_background == 0){
        currCommand->background = 0;
    }

    
    spawnpid = fork();
    switch (spawnpid){
        case -1:
            perror("Error Forking");
            exit(1);
            break;
        // child fork
        case 0:
            if(currCommand->background == 1){
                backgroundRedir(currCommand); // redirect stdin and stdout if the user hasn't already.
            }
            // child process if ran in foreground
            else{
                SIGINT_action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &SIGINT_action, NULL);
            }
            handleRedir(currCommand); // see if there are any redirections
           
            execvp(currCommand->user_command, currCommand->user_args); // execute the first command and pass in the arguments.
            printf("ERROR, Command not found\n"); // if command does not exist
            exit(1);
        break;

        // parent fork
        default:
            if(currCommand->background == 1){ // if it is a background process we will not wait for it. 
                printf("background pid is %d\n", spawnpid);
            }
            else{
                // wait for child process to finish and clean up.
                waitpid(spawnpid, &childExitMethod, 0);
                // status returned from child process
                status = childExitMethod;
                // check to see if it was killed by a signal.
                if(WTERMSIG(status) != 0){
                    printf("terminated by signal %d\n", WTERMSIG(status));
                }
            }
    }
}
// go through all the child processes and wait for them to finish while still allowing user to be prompted.
void cleanBackgroundProcess(){
    int spawnpid, childExitMethod;
    do{
        spawnpid = waitpid(-1, &childExitMethod, WNOHANG);
        // if the process exited then check to see how and set/print the exit status
        if (spawnpid > 0){
            // exited successfully
            if (WIFEXITED(childExitMethod) != 0){
				printf("background pid %d exited with value %d\n", spawnpid, WEXITSTATUS(childExitMethod));
            }
            // terminated by a signal
			else if (WIFSIGNALED(childExitMethod) != 0){
				printf("background pid %d exited and was terminated by signal %d\n", spawnpid, WTERMSIG(childExitMethod));
            } 
		
        }
    } while (spawnpid > 0);
}

int main()
{

    while(1){
        char userInput[MAX_CHARS];
        struct command *currCommand = malloc(sizeof(struct command));

        cleanBackgroundProcess();

        
        // display prompt
        printf(": ");
        fgets(userInput, MAX_CHARS, stdin);

        // check comments or blank lines
        if(userInput[0] == '#' || userInput[0] == '\n'){
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
