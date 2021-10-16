#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#define PREFIX "movies_"

struct movie
{
	char *title;
	char *year;
	char *lang;
	char *rating;
	struct movie *next;
};

char* findLargestFile()
{
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    struct stat statbuf;
    char *fileName;
    int max_length = 0;
    char *max_lengthFile = NULL;

    while((aDir = readdir(currDir)) != NULL)
    {
        fileName = aDir->d_name;
        if(strncmp(PREFIX, fileName, strlen(PREFIX)) == 0)
        {
            // check to see if it has a .csv extension
            if(strstr(fileName, ".csv") != NULL)
            {
                stat(fileName, &statbuf);
                if(max_length < statbuf.st_size)
                {
                    max_length = statbuf.st_size;
                    max_lengthFile = fileName;
                }
            }
        }
    }
    return max_lengthFile;
}

char* findSmallestFile()
{
    DIR *currDir = opendir(".");
    struct dirent *aDir;
    struct stat statbuf;
    char *fileName;
    int min_length = 0;
    char *min_lengthFile = NULL;
    int i = 0;

    // Go through all directories until we reached the end
    while((aDir = readdir(currDir)) != NULL)
    {
        fileName = aDir->d_name;

        // check to see if te file name has the "movies_" prefix
        if(strncmp(PREFIX, fileName, strlen(PREFIX)) == 0)         
        {
            // check to see if it has a .csv extension
            if(strstr(fileName, ".csv") != NULL)
            {
                stat(fileName, &statbuf);

                // find the file with the smallest size. 
                if(min_length > statbuf.st_size || i==0)
                {
                    min_length = statbuf.st_size;
                    min_lengthFile = fileName;
                    // int fileLength = statbuf.st_size;
                    i++;
                }
            }
        }
    }
    return min_lengthFile;
}

char* getUserFile()
{
    char userFile[30];
    memset(userFile, '\0', 30);
    char *fileName = NULL;
    printf("Please enter a file name: ");
    scanf("%s", userFile);

    char *currentFileName;

    DIR *currDir = opendir(".");
    struct dirent *aDir;
    while((aDir = readdir(currDir)) != NULL)
    {
        currentFileName = aDir->d_name;
        if(strcmp(currentFileName, userFile) == 0)
        {
            fileName = aDir->d_name;
        } 
    }
    return fileName;
}

void writeFile(struct movie *list, int year, int fd)
{
    char* title = list->title;
    strcat(title, "\n");
    write(fd, title, strlen(title));
}

void createFile(char *dirName, struct movie *list)
{
    
    char *newFilePath;
    char filePath[50];

    while(list!=NULL)
    {
        newFilePath = list->year;
        strcat(newFilePath, ".txt");
        memset(filePath, '\0', 50);

        sprintf(filePath, "./%s/%s", dirName, newFilePath);
        int fd = open(filePath, O_RDWR | O_CREAT | O_APPEND, 0640);

        writeFile(list, (long int) list->year, fd);

        list = list->next;
    }

}

void createDir(char *fileName, struct movie *list)
{
    //create a directory with name(chaplygy.movies.random) and message. Set permission to rwx r-x ---.
    int fileName_length = 21;
    char name[] = "chaplygy.movies.";
    srand(time(NULL));
    long int randomInt = rand() % 100000;
    
    char dirName[fileName_length];

    sprintf(dirName, "%s%lu", name, randomInt);
    mkdir(dirName, 0750);
    printf("created a directory with the name %s\n", dirName);

    createFile(dirName, list);
}

struct movie* createMovie(char *currLine)
{
    // create and allocate memory for the struct.
    struct movie *currMovie = malloc(sizeof(struct movie));

    // for use with strtok_r "in order to maintain context between successive calls that parse the same string." - linux man page
    char *saveptr;
    // title
    char *token = strtok_r(currLine, ",", &saveptr);            
    currMovie->title = calloc(strlen(token) + 1, sizeof(char)); 
    strcpy(currMovie->title, token);

    // year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token);

    //lang 
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->lang = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->lang, token);

    //rating
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rating = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->rating, token);

    currMovie->next = NULL;
    return currMovie;
}

struct movie *parseData(char *filePath)
{
    FILE *movieFile = fopen(filePath, "r");
    if(movieFile == NULL)
    {
        printf("error opening file\n");
    }

	char *currLine = NULL;
	size_t len = 0;
	ssize_t nread;
    int count = -1;
	
    // Set the head and tail of struct to point at NULL
	struct movie *head = NULL;
	struct movie *tail = NULL;

    // Go through whole file 
	while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
        // only if its not the first line(not the header), make a linked list
        if (count !=-1)
        {
            struct movie *newNode = createMovie(currLine);
            if (head == NULL)
            {
                head = newNode;
                tail = newNode;
            }
            else
            {
                tail->next = newNode;			
                tail = newNode;
            }
        }
        count++;        
	}

	free(currLine);
	fclose(movieFile);
	return head;
}

char* usersOptions()
{
    int userChoice2;
    char *fileName = NULL;

    do
    {
        printf("\nWhich file you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\nEnter a choice from 1 to 3: ");
        scanf("%d", &userChoice2);

        switch(userChoice2)
        {
            case 1: 
                fileName = findLargestFile();
                break;
            case 2: 
                fileName = findSmallestFile();
                break;
            case 3: 
                fileName = getUserFile();
                if(fileName == NULL)
                {
                    printf("File not found. Try again.\n");
                }
                break;
        }
    }while(fileName == NULL);
    
    printf("Now processing the chosen file named %s\n", fileName);
    return fileName;
}

int main(int argc, char *argv[])
{
    int userChoice;
    char *fileName;
    while(userChoice != 2)
    {
        printf("1. Select file to process\n2. Exit the program\n\nEnter a choice 1 or 2: ");
        scanf("%d", &userChoice);

        switch(userChoice)
        {
            case 1:
                fileName = usersOptions();
                struct movie *list = parseData(fileName);
                createDir(fileName, list);
                break;
            case 2:
                break;
            default:
                printf("incorrect option\n");
                break;
        }
         
    }

    
    return 0;
}
