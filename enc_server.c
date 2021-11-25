#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h> 
#define BUFFER_SIZE 512

void error(const char *msg) { 
  perror(msg); 
  exit(0); 
}
void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    memset((char*) address, '\0', sizeof(*address)); 
    address->sin_family = AF_INET;

    address->sin_port = htons(portNumber);

    address->sin_addr.s_addr = INADDR_ANY; // set as localhost target IP address instead??
}

char* encryptData(char* string){
    // CHANGE THE BUFFER SIZE TO MAKE SURE IT FITS THE ORIGINAL STRING
    char encryptedString[BUFFER_SIZE];
    // get the key from the string (it is in between the special characters @@)
    char* key = strstr(string, "@@") +2;
    key[strcspn(key, "@@")] = '\0'; // remove special marker
    
    // remove special markers from the original string
    string[strcspn(string, "@@")] = '\0';
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
            if(charKey == 32){
                charKey = 26;
                charString -= 65;
            }
            if(charString == 32){
                charString = 26;
                charKey -= 65;
            }
            if(charKey == 32 && charString == 32){
                charString = 26;
                charKey = 26;
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
    // overwrite the plaintext string and return. 
    strcpy(string, encryptedString);
    return string;
}

int main(int argc, char* argv[]){

    int listenSocket, portNumber, charsRead, connectionSocket;
    char buffer[BUFFER_SIZE];
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
        } 
        //else(error) 


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
                    // revc client data(plaintext & key)
                    // Get the message from the client and display it
                    memset(buffer, '\0', BUFFER_SIZE);
                    // Read the client's message from the socket
                    charsRead = recv(connectionSocket, buffer, BUFFER_SIZE, 0); 
                    if (charsRead < 0){
                        error("ERROR reading from socket");
                    }
                    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
                    // ----------------------------------------------------------
                    // do encryption
                    char *encryptedString = encryptData(buffer);
                    // printf("%s\n", encryptedString);
                    // ----------------------------------------------------------
                    
                    // write back ciphertext to enc_client
                    // Send a Success message back to the client
                    charsRead = send(connectionSocket, encryptedString, strlen(encryptedString), 0); 
                    if (charsRead < 0){
                    error("ERROR writing to socket");
                    }
                    // Close the connection socket for this client
                    close(connectionSocket);
                    exit(0); // break out of the while loop??
                    break;
                // parent fork
                default:
                waitpid(spawnpid, &childExitMethod, 0);
                printf("Child exit status: %d", childExitMethod);  
                fflush(stdout);              
            }
    }

    close(listenSocket);
    return 0;
}
