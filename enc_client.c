#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // for gethostbyname

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in *address, int portNumber){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname("localhost");
    if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

}

int main(int argc, char* argv[]){
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    // check if we recieved the correct cmd args
    if (argc < 4){fprintf(stderr,"USAGE: %s plaintextFile key port\n", argv[0]); exit(0);} 
    
    // open plaintext file and key file
    FILE* fp = fopen(argv[1], "r");
    FILE* fp2 = fopen(argv[2], "r");

    // check to see if files exist
    if (fp == NULL || fp2 == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    // compare the stringBuffer to the length of keygen
    fseek(fp, 0, SEEK_END);
    fseek(fp2, 0, SEEK_END);
    
    long int textFileSize = ftell(fp);
    long int keyFileSize = ftell(fp2);

    if(keyFileSize < textFileSize){
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }
    // set fp back to beginning of file.
    rewind(fp);
    rewind(fp2);

    // needed to get full string.
    textFileSize++;
    keyFileSize++;
    // create char array to hold the plaintext and key files
    char stringBuffer[textFileSize];
    char keyBuffer[textFileSize];

    // clear out buffers
    memset(stringBuffer, '\0',textFileSize);
    memset(keyBuffer, '\0', textFileSize);

    // Get input from the user
    fgets(stringBuffer, textFileSize, fp);
    fgets(keyBuffer, textFileSize, fp2);
    // printf("stringBuffer %s\n", stringBuffer);
    // printf("keyBuffer %s\n", keyBuffer);
    
    // check for any bad characters. (characters not in ASCII 60 to 90, space 32, 
    // nor the newline 10)
    for(int i = 0; i<strlen(stringBuffer); i++){
        if(stringBuffer[i] < 65 || stringBuffer[i] > 90  ){
            // is not the space character or newline
            if(stringBuffer[i] != 32 && stringBuffer[i] != 10){
                //print error
                fprintf(stderr, "Error: Input contains bad characters\n");
                exit(1);
            }
        }
    }
    
    // Add special marker to know its the end of the plaintext input
    // Remove the trailing \n that fgets adds 
    int newLineIndex = strcspn(stringBuffer, "\n");
    stringBuffer[newLineIndex] = '!'; 
    stringBuffer[newLineIndex+1] = '!'; 
    stringBuffer[newLineIndex+2] = '\0'; // might not need
    // add special marker to know its the end of the key
    newLineIndex = strcspn(keyBuffer, "\n");
    keyBuffer[newLineIndex] = '@'; 
    keyBuffer[newLineIndex+1] = '@'; 
    keyBuffer[newLineIndex+2] = '\0'; // might not need
    
    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    portNumber = atoi(argv[3]);     // port number into int
    // Set up the server address struct
    setupAddressStruct(&serverAddress, portNumber);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){error("CLIENT: ERROR connecting");}

    // check to make sure its the enc_server
    // send a 1 to tell them its the client
    char checkBuf[2];
    memset(checkBuf, '\0', 2);
    charsWritten = send(socketFD, "1", 1, 0);
    if(charsWritten < 1){
        fprintf(stderr, "Error writing to socket\n");
        exit(2);
    }else{
        // recieve a message back from the client to know we are connected
        charsWritten = recv(socketFD,checkBuf,2,0);
        if(strncmp(checkBuf, "OK", 2) != 0 || charsWritten < 0){exit(0);}
    }

    // Send message to server
    int count=0;
    do{
        // send the plaintext file
        charsWritten = send(socketFD, stringBuffer+count, 1000, 0); 
        if (charsWritten < 0){error("CLIENT: ERROR writing to socket");}
        count += charsWritten;
    }while(count < strlen(stringBuffer));

    // make sure the server is ready for the key
    memset(checkBuf, '\0', 2);
    charsWritten = recv(socketFD, checkBuf, 2,0);
    if(strncmp(checkBuf, "OK", 2) != 0 || charsWritten < 0){printf("Client did not recieve OK\n");exit(0);}
    
    // send the key. 
    // printf("\n\n\n\nkey: %s\n\n", keyBuffer);
    count = 0;
    do{
        charsWritten = send(socketFD, keyBuffer+count, 1000, 0); 
        if (charsWritten < 0){error("CLIENT: ERROR writing to socket");}
        count += charsWritten;
    }while(count< strlen(keyBuffer));
    
    // make sure we the client is ready for the ciphertext 
    memset(checkBuf, '\0', 2);
    charsWritten = recv(socketFD, checkBuf, 2,0);
    if(strncmp(checkBuf, "OK", 2) != 0 || charsWritten < 0){printf("Client did not recieve OK\n");exit(0);}

    // Clear out the buffer again for reuse
    memset(stringBuffer, '\0', sizeof(stringBuffer));
    // memset(keyBuffer, '\0', sizeof(keyBuffer));
    // Read data from the socket
    char temp[textFileSize];
    memset(temp, '\0', textFileSize);

    do{
        charsRead = recv(socketFD, temp, strlen(keyBuffer) , 0); 
        if (charsRead < 0){error("CLIENT: ERROR reading from socket");}
        strcat(stringBuffer, temp);
    }while(strstr(stringBuffer, "@@") ==NULL);
    stringBuffer[strcspn(stringBuffer, "@@")] = '\0';
    // printf("client recivedlen: %d\n", (int)strlen(stringBuffer));

    // let server know we have recived all data
    send(socketFD, "OK", 2, 0);
    // ----------------------------------------------------------------------------
    //    (charsRead < strlen(stringBuffer));   // check to see if the length of charsRead matches the length of the original string from the txt file
    // printf("CLIENT: I received this from the server: \"%s\"\n", stringBuffer);

    // send to stdout here
    write(1, stringBuffer, strlen(stringBuffer));
    write(1, "\n", 1);
    // Close the socket
    close(socketFD); 
    // Close the files
    fclose(fp);
    // remove(argv[2]);
    fclose(fp2);
    return 0;
}
