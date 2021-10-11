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

// Create a movie struct with tokens from the current line being read.
// set each token to the corresponding data in the struct
struct movie *createMovie(char *currLine)
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

// Create a linked list structure of each of the movies
struct movie *processFile(char *filePath)
{

	FILE *movieFile = fopen(filePath, "r");

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

int userChoice()
{
    printf("\n1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\nEnter a choice from 1 to 4: ");
    int choice;
    scanf("%d", &choice);
    return choice;
}

// Print the highest rating of each movie per year
void printRating(struct movie *currHigh, int arr[], int movie_count)
{
    // Used to see if we already printed that movie
    int check = 0;

    // Go through all lines
    for(int i=0; i<movie_count;i++)
    {
        if (atoi(currHigh->year) == arr[i])
        {
            // Dont print if the year is in the array already 
            check = -1;
        }

    }

    // print the movie since if it has not been printed 
    if (check == 0)
    {
        printf("%d, %.1f, %s\n", atoi(currHigh->year), strtod(currHigh->rating, NULL), currHigh->title);
    }

}

// Getting the data for the highest movie of each year
void highestRating(struct movie *list, int movie_count)
{
    struct movie *currentMovie = list;
    struct movie *currHigh = currentMovie;
    struct movie *nextMovie = currentMovie;

    // Create array to store the years that already have been printed
    int years[movie_count];

    // The current line/movie will be the other loop
    for (int i=0; i<movie_count-1; i++)
    {
        
        nextMovie = list;
        currHigh = currentMovie;

        // check all other movies to see of their rating is higher
        for(int j=0; j<movie_count; j++)
        {
            if (nextMovie == NULL)
            {
                break;
            }

            // Look for a movie with the same year
            if (atoi(currHigh->year) == atoi(nextMovie->year))
            {
                // Compare the ratings
                if(strtod(currHigh->rating, NULL) < strtod(nextMovie->rating, NULL))
                {
                    currHigh = nextMovie;                
                }
            }
            // Go to next movie
            nextMovie = nextMovie->next;
        }

        // Call this function to check if movie has been printed and print if not
        printRating(currHigh, years, movie_count);
        // Add to the array that the movie was printed
        years[i] = atoi(currHigh->year);

        // Go to next movie
        currentMovie = currentMovie->next;
        }
}

// See if the users choice of year matches any movie
// Print if it does
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

// Used to parse the language of the struct to get the individual language
int tokenize(struct movie *list, char *currLine, char lang[], int check)
{  
    char *saveptr;
    char *token;
    
    // parse until reached the ";"
    token = strtok_r(currLine, ";", &saveptr);
    
    // keep parsing until reached the end of the string
    while (token != NULL)
    {
        if (strcmp(token, lang) == 0)
        {
            // print if we have found the language and set the check to know it exists 
            printf("%s %s\n", list->year, list->title);
            check = 0;
        }
        token = strtok_r(NULL, ";", &saveptr);
    }
    return check;        
}

// Parse each language in the file and check if that language matches what the user entered
int printLang(struct movie *list, char lang[], int movie_count)
{
    struct movie *currentMovie;
    currentMovie = list;
    char *currLine;
    char *saveptr;
    int check = -1;

    for (int i=0; i< movie_count; i++)
    {
        currLine = currentMovie->lang;
        currLine = strtok_r(currLine+1, "]", &saveptr);
        check = tokenize(currentMovie, currLine, lang, check);
        
        currentMovie = currentMovie->next;
    }
    return check;
    
}

// Get the number of moives in the file and return that number
int getCount(char *filePath)
{
    FILE *movieFile = fopen(filePath, "r");

	char *currLine = NULL;
	size_t len = 0;
    ssize_t nread;
    int count = -1;
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
        count++;        
	}
    
    free(currLine);
	fclose(movieFile);

    return count;
}


int main(int argc, char *argv[])
{
    int choice = 0;
    int movie_count = -1;

    if (argv[1] == NULL)
    {
        printf("You did not include a file to read\n");
        return -1;
    }
    // Do this while the choice is not 4.
    while (choice != 4)
    {

        // Present prompt to user and allow them to make a choice.
        choice = userChoice();

        // Make a movie struct with list pointed at the head of the linked list
        struct movie *list = processFile(argv[1]);
        movie_count = getCount(argv[1]);
        printf("\nProcessed file %s and parsed data for %d movies\n",argv[1], movie_count);

        // Call functions with user choice
        switch(choice)
        {
            case 1:
                printf("Enter the year for which you want to see movies: ");
                int year;
                scanf("%d", &year);
                // Call a function to go through linked list and movie data and print titles of movies.
                if((printTitle(list, year)) == 1)
                {
                    printf("No movies with that year found.");
                }
                break;
            case 2:
                highestRating(list, movie_count);
                break;
            case 3:
                printf("Enter the language for which movies you want to see in that language: ");
                char lang[20];
                scanf("%s", &lang);
                if((printLang(list, lang, movie_count)) == -1)
                {
                    printf("No data about movies released in %s", lang);
                }
                break;
            case 4:
                break;
            default:
                printf("You entered an incorrect choice. Try again.\n");
        }
    }
	return 0;
}
