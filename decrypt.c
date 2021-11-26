char* decryptData(char* string){
    char decryptedString[BUFFER_SIZE];
    char* key = strstr(string, "@@") +2;
    // get the key from the string (it is in between the special characters @@)
    key[strcspn(key, "@@")] = '\0'; // remove special marker
    
    // remove special markers from the original string
    string[strcspn(string, "@@")] = '\0';

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
