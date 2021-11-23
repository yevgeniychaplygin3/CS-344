#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
    }
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

}

int main(int argc, char* argv[]){
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    // Set up for getting user input------------------------------------
    if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintextFile key port\n", argv[0]); 
    exit(0); 
    } 
    FILE* fp = fopen(argv[1], "r");
    FILE* fp2 = fopen(argv[2], "r");
    // CHECK TO SEE IF FILES ARE FOUND OR NOT. 
    if (fp == NULL || fp2 == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    // char *plaintext = NULL;
    char stringBuffer[256];
    char keyBuffer[256];


    memset(stringBuffer, '\0', sizeof(stringBuffer));
    memset(keyBuffer, '\0', sizeof(keyBuffer));
    // Get input from the user, trunc to buffer - 1 chars, leaving \0
    // maybe use a while loop to get all the data if it's larger than 256 and allocate more space to read.
    fgets(stringBuffer, sizeof(stringBuffer)-1, fp);
    // Remove the trailing \n that fgets adds
    stringBuffer[strcspn(stringBuffer, "\n")] = '\0'; 
    printf("stringBuffer %s\n", stringBuffer);
    //---------------------------------------------------------------------------
    
    // compare the stringBuffer to the length of keygen
    fseek(fp, 0, SEEK_END);
    fseek(fp2, 0, SEEK_END);
    if(ftell(fp2) < ftell(fp)){
        printf("Key file is not long enough for textfile\n");
        // RETURN ERROR
    }
    
    
    
    portNumber = atoi(argv[3]);
    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }
    // Set up the server address struct
    setupAddressStruct(&serverAddress, portNumber);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");

    }
    // Send message to server
    // Write to the server
    charsWritten = send(socketFD, stringBuffer, strlen(stringBuffer), 0); 
    if (charsWritten < 0){
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(stringBuffer)){
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    // Clear out the stringBuffer again for reuse
    memset(stringBuffer, '\0', sizeof(stringBuffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, stringBuffer, sizeof(stringBuffer) - 1, 0); 
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    printf("CLIENT: I received this from the server: \"%s\"\n", stringBuffer);

    // Close the socket
    close(socketFD); 
    fclose(fp);
    return 0;
}
