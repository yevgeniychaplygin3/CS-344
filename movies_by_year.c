#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define PREFIX movies

void findLargestFile()
{
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    struct stat statbuf;

    // SELF CHECKING LENGTH OF FILE
    // int len;
    // FILE *fp = fopen("movies_sampleLong_1.csv", "r");
    // if (fp == NULL){
    //     printf("error opening");
    // }

    // fseek(fp,0,SEEK_END);
    // len = ftell(fp);
    // fclose(fp);
    // printf("%d bytes", len);


    char *fileName;
    while((aDir = readdir(currDir)) != NULL)
    {
        if(strncmp("movies", aDir->d_name, strlen("movies")) == 0)          // STOPPED HERE. WE GOT THE FILE NAME NOW WE JUST HAVE TO GET THE FILE SIZES AND MAYBE COMPARE TO SEE WHICH IS THE LARGEST.. CREATE A NEW FUNCTION THE SAME AS THIS ONE EXCEPT GET THE SMALLEST. ALSO CREATE ANOTHER FUNCTION TO GET A FILE NAME FROM THE USER.
        {
            stat(aDir->d_name, &statbuf);
            fileName = aDir->d_name;
            printf("%s\n", fileName);
            // if()
        }
        // printf("%s %lu\n", aDir->d_name, aDir->d_ino);
    }
    // char *filePath = aDir->d_name;
    // printf("%s", filePath);
    // FILE *largeFile = fopen(filePath, "r");
    // return largeFile;
}

// void usersOptions()
// {
//     int userChoice2;
//     printf("Which file you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\nEnter a choice from 1 to 3: ");
//     scanf("%d", &userChoice2);
//     FILE *fileName;

//     switch(userChoice2)
//     {
//         case 1: 
//             fileName = findLargestFile();
//             break;
//         case 2: 
//             // fileName = findSmallestFile();
//             break;
//         case 3: 
//             // fileName = getUserFile();
//             break;
//     }
//     printf("Now processing the chosen file named %s", (char*)fileName);

// }

int main(int argc, char *argv[])
{
    findLargestFile();
    // int userChoice;
    // while(userChoice != 2)
    // {
    //     printf("1. Select file to process\n2. Exit the program\nEnter a choice 1 or 2: ");
    //     scanf("%d", &userChoice);

    //     switch(userChoice)
    //     {
    //         case 1:
    //             usersOptions();
    //             break;
    //         case 2:
    //             break;
    //         default:
    //             printf("incorrect option\n");
    //             break;
    //     }
    // }



    return 0;
}