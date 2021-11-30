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

    address->sin_addr.s_addr = INADDR_ANY;
}

//decrypt data 
char* decryptData(char* string, char* key){
    // ------------------------------------------------------------------------
    
    // char* key = strstr(string, "!!") +2;
    // // get the key from the string (it is in between the special characters @@)
    // key[strcspn(key, "@@")] = '\0'; // remove special marker
    
    // // remove special markers from the original string
    // string[strcspn(string, "@@")] = '\0';
    // ------------------------------------------------------------------------
    char decryptedString[BUFFER_SIZE];
    // start decryption
    int encryptedStringNum;
    int charKey;
    int originalChar;
    for(int i=0; i<strlen(string); i++){
        encryptedStringNum = string[i];
        charKey = key[i];

        if(encryptedStringNum == 32){
            encryptedStringNum = 26;
        }else if(charKey == 32){
            charKey = 26;
            encryptedStringNum -= 65;
        }    
        else{
            encryptedStringNum -= 65;
            charKey -= 65;
        }
        originalChar = (encryptedStringNum-charKey)%27;

        if(originalChar < 0){
            originalChar += 27; // add to make it not negative
        }

        if(originalChar == 26){ // convert back to ASCII 32, a space.
            originalChar = 32;
        }else{
            originalChar += 65; // else convert back to ASCI 65-90 values
        }
        // place the original character into the decrypted string buffer.
        decryptedString[i] = (char) originalChar;
    }
    // overwrite the encrypted string with the decrypted string and return.
    strcpy(string, decryptedString);
    return string;
}

int main(int argc, char* argv[]){

    int listenSocket, portNumber, charsRead, charsWritten, connectionSocket;

    char buffer[BUFFER_SIZE];
    char key[BUFFER_SIZE-3];
    char temp[BUFFER_SIZE];

    memset(buffer, '\0', BUFFER_SIZE);
    memset(temp, '\0', BUFFER_SIZE);
    memset(key, '\0', BUFFER_SIZE);
    
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    // char bufferPtr[BUFFER_SIZE];
    // char keyBuffer[BUFFER_SIZE];
    // memset(bufferPtr, '\0', BUFFER_SIZE);
    // memset(keyBuffer, '\0', BUFFER_SIZE);

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
        // check to see if its communicating with dec_client
        //if(we recive a 0 than its the dec_client)
        char checkClient[] = "-5";
        int check = recv(connectionSocket, checkClient, 1,0); 
        // printf("checkClient: %s\n", checkClient);
        if(strncmp(checkClient, "0", 1) != 0 || check < 0){
            fprintf(stderr, "Could not connect to dec_server on port %d. Disconnecting now..\n", portNumber);
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
                    // revc client data(plaintext) until we do not see the special marker
                    do{
                        charsRead = recv(connectionSocket, temp, 1000, 0);
                        if (charsRead < 0){error("ERROR reading from socket");}
                        // printf("temp: %s", temp);
                        strcat(buffer, temp);
                    }while(strstr(buffer, "!!") == NULL);
                    
                    // strip off the special marker
                    buffer[strcspn(buffer, "!!")] = '\0';

                    // send to let the client know we got the text
                    charsRead = send(connectionSocket, "OK", 2,0);
                    if (charsRead < 0){error("ERROR writing to socket");}
                    
                    // recv client key until we do not see the special marker
                    do{
                        charsRead = recv(connectionSocket, temp, 1000, 0); 
                        if (charsRead < 0){error("ERROR reading from socket");}
                        strcat(key, temp);
                        
                    }while(strstr(key, "@@") == NULL);
                    
                    // strip off the special marker
                    key[strcspn(key, "@@")] = '\0';

                    // send to let the client know we got the key
                    charsRead = send(connectionSocket, "OK", 2,0);
                    if (charsRead < 0){error("ERROR writing to socket");}
                    
                    //do decryption
                    char *decryptedString = decryptData(buffer, key);

                    // send ciphertext to client
                    int count = 0;
                    do{
                        charsWritten = send(connectionSocket, decryptedString+count, 1000, 0); 
                        if (charsWritten < 0){error("ERROR writing to socket");}
                        count +=charsWritten; 
                    }while(count<strlen(decryptedString));

                    // wait for client to receive all data
                    char finish[2]; 
                    while(strcmp(finish, "OK") != 0)
                    {
                        recv(connectionSocket, finish, 2, 0);
                    }

                    // Close the connection socket for this client
                    close(connectionSocket);
                    exit(0); // break out of the while loop
                    break;
                // parent fork
                default:
                // clean up the children
                waitpid(spawnpid, &childExitMethod, 0);
            }
    }

    close(listenSocket);
    return 0;
}
