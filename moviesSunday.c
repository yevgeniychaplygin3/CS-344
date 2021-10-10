#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// count for # of movies. 
static int count = -1;

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
        count = 24;        
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

int userChoice()
{
    printf("\n1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\nEnter a choice from 1 to 4: ");
    int choice;
    scanf("%d", &choice);
    return choice;
}

//stopped here. Want to see if the movie has already been printed or not in the years array
void printRating(struct movie *currHigh)
{
    int years[24];
    for(int i=0; i<sizeof(years);i++)
    {
        if (currHigh->year == years[i])
        {
            printf("%d, %.1f, %s\n", atoi(currHigh->year), strtod(currHigh->rating, NULL), currHigh->title);
        }
    }

}

// STOPPED HERE, try to not print duplicates. line 149
void highestRating(struct movie *list)
{
    struct movie *currentMovie = list;
    struct movie *currHigh = currentMovie;
    struct movie *nextMovie = currentMovie;


    // place main/current movie here
    for (int i=0; i<count-1; i++)
    {
        
        // here we look for a movie with the same year
        nextMovie = list;
        currHigh = currentMovie;
        for(int j=0; j<count; j++)
        {
            if (nextMovie == NULL)
            {
                break;
            }

            if (atoi(currHigh->year) == atoi(nextMovie->year))
            {
                if(strtod(currHigh->rating, NULL) < strtod(nextMovie->rating, NULL))
                {
                    currHigh = nextMovie;                
                }
            }
            nextMovie = nextMovie->next;
        }
        // printf("%d, %.1f, %s\n", atoi(currHigh->year), strtod(currHigh->rating, NULL), currHigh->title);
        printRating(currHigh);
        currentMovie = currentMovie->next;

        }
}

int printTitle(struct movie *list, int year)
{
    int check = 1;
    while(list != NULL)
    {
        if(atoi(list->year) == year)
        {
            printf("%s\n", list->title);
            check = 0;
        }
        list = list->next;
     }
     return check;
}


void tokenize(struct movie *list, char *currLine, char lang[])
{  
    char *saveptr;
    char *token;
    
    token = strtok_r(currLine, ";", &saveptr);
    while (token != NULL)
        {
            if (strcmp(token, lang) == 0)
            {
                printf("%s %s\n", list->year, list->title);
            }
            // currLine = saveptr;
            token = strtok_r(NULL, ";", &saveptr);

        }
}
// trying not get currLine to change when i set the token
void printLang(struct movie *list, char lang[])
{
    struct movie *currentMovie;
    currentMovie = list;
    char *currLine;
    char *saveptr;


    for (int i=0; i<count; i++)
    {
        currLine = currentMovie->lang;
        currLine = strtok_r(currLine+1, "]", &saveptr);
        tokenize(currentMovie, currLine, lang);
        
        currentMovie = currentMovie->next;
      

        // while (token != NULL)
        //     {
        //         if (strcmp(token, lang) == 0)
        //         {
        //             printf("%s %s\n", list->year, list->title);
        //         }
        //         // currLine = saveptr;
        //         token = strtok(NULL, ";");

        //     }
    }
    
}



int main(int argc, char *argv[])
{
    int choice = 0;
    while (choice != 4)
    {

        // Present prompt to user and allow them to make a choice.
        // Do this while the choice is not 4.
        choice = userChoice();
        struct movie *list = processFile(argv[1]);
        switch(choice)
        {
            case 1:
                printf("Enter the year for which you want to see movies: ");
                int year;
                scanf("%d", &year);
                //call a function to go through linked list and movie data and print titles of movies.
                if((printTitle(list, year)) == 1)
                {
                    printf("No movies with that year found.");
                }
                break;
            case 2:
                highestRating(list);
                break;
            case 3:
                printf("Enter the language for which movies you want to see in that language: ");
                char lang[20];
                scanf("%s", &lang);
                printLang(list, lang);
                break;
            case 4:
                break;
            default:
                printf("You entered an incorrect choice. Try again.\n");

        }
    }
    // printMovieList(list);
	return 0;
}
