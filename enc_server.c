#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h> 
#define BUFFER_SIZE 70000

void error(const char *msg) { 
  perror(msg); 
  exit(0); 
}
void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 
    // The address should be network capable. AF_INET is internet. vs. AF_UNIX is local.
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    address->sin_addr.s_addr = INADDR_ANY; // change to localhost
}

char* encryptData(char* string, char *key){
    
    char encryptedString[strlen(string)];
    // printf("here1\n");
    // printf("%s\n", string);
    // printf("%s\n", key);
    
    // start encrypting

    int charString, charKey;

    /* This for loop will first change the ASCII values of each character
    to a scale of 0-26. Where 0=A, 1=B and so on.. 26 is the space character.
    this is done so we can mod 27 when we add the plaintext and the key.*/
    for(int i=0; i<strlen(string); i++){
        // convert to int
        charString = string[i];
        charKey = key[i];

        // check for spaces. If character is a space, then place as last index(26)
        // will subtract all other characters by 65 to convert to 0 to 26 scale
        if(charKey == 32 || charString == 32){
            if(charKey == 32 && charString == 32){
                charString = 26;
                charKey = 26;
            }
            if(charKey == 32){
                charKey = 26;
                charString -= 65;
            }
            if(charString == 32){
                charString = 26;
                charKey -= 65;
            }
            
        }else{ // neither character is a space (32)
            charKey -= 65;
            charString -= 65;
        }

        // add and mod 27 to recieve the new encrypted character
        int newChar = (charString + charKey) % 27; 
        // if its the last index on our scale, then it is the space character
        if(newChar == 26){
            newChar = 32;
        }else{ // otherwise add 65 to convert back to ASCII
            newChar += 65;
        }
        // place the newChar into the encrypted string
        encryptedString[i] = (char) newChar;
        
    }
    // printf("here2\n");

    // overwrite the plaintext string and return. 
    strcpy(string, encryptedString);
    return string;
}

int main(int argc, char* argv[]){

    int listenSocket, portNumber, charsRead, connectionSocket;
    char buffer[BUFFER_SIZE];
    int n;

    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    if(argc < 2){ 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    }

    // socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0){
        error("CLIENT: ERROR opening socket");
    }

    portNumber = atoi(argv[1]);
    setupAddressStruct(&serverAddress, portNumber);

    // bind
    if(bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }
    // listen
    listen(listenSocket, 5);

    // accept using while loop
    while(1){

        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }
        // check to make sure its the enc_client.
        //if(we recive a 1 than its the client)
        char checkClient[] = "0";
        int check = recv(connectionSocket, checkClient, 1,0); 
        if(strcmp(checkClient, "1") != 0 || check < 0){
            fprintf(stderr, "Could not connect to enc_server on port %d. Disconnecting now..\n", portNumber);
            close(listenSocket);
            close(connectionSocket);

            exit(2);
        }else{
            // let the client know that we are connected
            send(connectionSocket, "OK", 2, 0);

        }

        // printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));

        // fork
            pid_t spawnpid = -5;
            int childExitMethod = -5;

            spawnpid = fork();
            switch(spawnpid){
                case -1:
                    perror("Error Forking");
                    break;
                // child fork
                case 0:
                    // revc client data(plaintext & key)
                    
                    n = 128;
                    char *bufferPtr, *keyBuffer;
                    bufferPtr = (char*) calloc(n, sizeof(char));
                    keyBuffer = (char*) calloc(n, sizeof(char));
                    if(bufferPtr == NULL || keyBuffer == NULL){
                        fprintf(stderr, "Insufficent memory for server buffer\n");
                        exit(0);
                    }

                    // have to parse out the data recived from client into 
                    // bufferPtr = plain text
                    // key = key
                    memset(buffer, '\0', BUFFER_SIZE);
                    do{
                        n += 128;
                        bufferPtr = realloc(bufferPtr, n*sizeof(char));
                        // Get the message from the client and display it
                        // Read the client's message from the socket
                        // printf("Server now recieving data.\n");
                        charsRead = recv(connectionSocket, buffer, BUFFER_SIZE, 0); 
                        if (charsRead < 0){
                            error("ERROR reading from socket");
                        }
                        // copy to bufferPtr
                        strcpy(bufferPtr, buffer);
                        // printf("SERVER: I received this from the client: \"%s\"\n", buffer);
                        // printf("bufferPtr: %s\n", bufferPtr);
                    }while(strstr(bufferPtr,"!!") == NULL);

                    bufferPtr[strcspn(bufferPtr, "!!")] = '\0'; // delete the special marker
                    // printf("out of while loop bufferPtr: %s\n", bufferPtr);

                    memset(buffer, '\0', BUFFER_SIZE);
                    do{
                        keyBuffer = realloc(keyBuffer, n*2);
                        // printf("Server now recieving data.\n");
                        charsRead = recv(connectionSocket, buffer, BUFFER_SIZE, 0); 
                        if (charsRead < 0){
                            error("ERROR reading from socket");
                        }
                        strcpy(keyBuffer, buffer);
                        // printf("SERVER: I received this from the client: \"%s\"\n", buffer);
                        // printf("keyBuffer: %s\n", keyBuffer);
                        

                    }while(strstr(keyBuffer, "@@") == NULL);

                    keyBuffer[strcspn(keyBuffer, "@@")] = '\0';
                    // printf("left while loop\n");
                    // printf("out of while loop keyBuffer: %s\n", keyBuffer);
                    
                    // do encryption
                    char *encryptedString = encryptData(bufferPtr, keyBuffer);

                        
                    // printf("now sending encryptedString: %s\n", encryptedString);
                    // fflush(stdout);
                    charsRead = send(connectionSocket, encryptedString, strlen(encryptedString), 0); 
                    if (charsRead < 0){error("ERROR writing to socket");}


                    // printf("%s\n", encryptedString);
                    
                    // write back ciphertext to enc_client // Send a Success message back to the client
                    

                    free(bufferPtr);
                    free(keyBuffer);
                    // Close the connection socket for this client
                    close(connectionSocket);
                    // printf("fork exiting..\n");
                    exit(0); // break out of the while loop??
                    break;
                // parent fork
                default:
                waitpid(spawnpid, &childExitMethod, 0);
                // printf("Child exit status: %d\n", childExitMethod);  
                // fflush(stdout);              
            }
    }

    close(listenSocket);
    return 0;
}
