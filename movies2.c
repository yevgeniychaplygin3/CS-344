#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct movie
{
	char *title;
	char *year;
	char *lang;
	char *rating;
	struct movie *next;
};


struct movie *createMovie(char *currLine)
{
    struct movie *currMovie = malloc(sizeof(struct movie));

    char *saveptr;
    // title
    char *token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // year
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token);

    //lang. Need to parse [lang; lang;...],
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

struct movie *processFile(char *filePath)
{
    // count for # of movies. 
    static int count =-1;
	
	// printf("\nProcessed file %s\n",filePath);
	FILE *movieFile = fopen(filePath, "r");

	char *currLine = NULL;
	size_t len = 0;
	ssize_t nread;
	

	struct movie *head = NULL;
	struct movie *tail = NULL;

	while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
        // only if its not the first line
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
	printf("\nProcessed file %s and parsed data for %d movies\n",filePath, count);
    
	
	free(currLine);
	fclose(movieFile);
	return head;
}

void printMovie(struct movie *aMovie)
{
    printf("%s, %s, %s, %s\n", aMovie->title, aMovie->year, aMovie->lang, aMovie->rating);
}

void printMovieList(struct movie *list)
{
    while(list != NULL)
    {
        printMovie(list);
        list = list->next;
    }
}


void userOptions()
{
    printf("\n1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\nEnter a choice from 1 to 4: ");
}

int userChoice()
{
    int choice;
    scanf("%d", &choice);
    return choice;
}

/* Stopped at getting user choices using switch statement and making functions for those choices*/
int main(int argc, char *argv[])
{
	// struct movie *list = processFile(argv[1]);
    userOptions();
    int choice = userChoice();

    switch(choice)
    {
        case 1:
            moviesInYear();
            break;
        case 2:

        case 3:
        case 4:
            break;
        default:
            printf("You entered an incorrect choice. Try again.\n");

    }
    // printMovieList(list);
	return 0;
}
