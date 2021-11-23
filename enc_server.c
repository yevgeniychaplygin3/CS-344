#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h> 

void error(const char *msg) { 
  perror(msg); 
  exit(0); 
}
void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    memset((char*) address, '\0', sizeof(*address)); 
    address->sin_family = AF_INET;

    address->sin_port = htons(portNumber);

    address->sin_addr.s_addr = INADDR_ANY;
}

char* encryptData(char* string){
    char* encryptedString = NULL;


    return encryptedString;
}

int main(int argc, char* argv[]){

    int listenSocket, portNumber, charsRead, connectionSocket;
    char buffer[256];
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
        printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));

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
                    // check to see if its communicating with enc_client
                    // revc client data(plaintext & key)


                    // Get the message from the client and display it
                    memset(buffer, '\0', 256);
                    // Read the client's message from the socket
                    charsRead = recv(connectionSocket, buffer, 255, 0); 
                    if (charsRead < 0){
                        error("ERROR reading from socket");
                    }
                    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
                    // ----------------------------------------------------------
                    // do encryption
                    encryptData(buffer);
                    // ----------------------------------------------------------
                    
                    // write back ciphertext to enc_client
                    // Send a Success message back to the client
                    charsRead = send(connectionSocket, "I am the server, and I got your message", 39, 0); 
                    if (charsRead < 0){
                    error("ERROR writing to socket");
                    }
                    // Close the connection socket for this client
                    close(connectionSocket);
                    break;
                // parent fork
                default:
                waitpid(spawnpid, &childExitMethod, 0);
                printf("Child exit status: %d", childExitMethod);                
            }
    }

    close(listenSocket);
    return 0;
}
