#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Constants
#define MAX_FIELD_LENGTH 256

// Struct for Book record (variable-length fields)
typedef struct
{
    char *bookTitle;
    char *authorName;
    char *isbn;
} BookRecord;

void readInput(char **input)
{
    char buffer[MAX_FIELD_LENGTH];
    int i = 0;
    char c;

    while (i < MAX_FIELD_LENGTH - 1 && (c = getchar()) != '\n' && c != EOF)
    {
        buffer[i++] = c;
    }

    buffer[i] = '\0';
    *input = strdup(buffer); // Allocate memory for the string and copy it.
    if (*input == NULL)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    if (c != '\n' && c != EOF)
    {
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

void freeRecord(BookRecord record)
{
    free(record.bookTitle);
    free(record.authorName);
    free(record.isbn);
}

// Function to write a variable-length record to a file
void writeRecord(FILE *file, BookRecord record)
{
    int titleLen = strlen(record.bookTitle);
    int authorLen = strlen(record.authorName);
    int isbnLen = strlen(record.isbn);

    // Write field lengths and the data
    fwrite(&titleLen, sizeof(int), 1, file);
    fwrite(record.bookTitle, sizeof(char), titleLen, file);

    fwrite(&authorLen, sizeof(int), 1, file);
    fwrite(record.authorName, sizeof(char), authorLen, file);

    fwrite(&isbnLen, sizeof(int), 1, file);
    fwrite(record.isbn, sizeof(char), isbnLen, file);
}
// Function to read a variable-length record from a file
BookRecord readRecord(FILE *file)
{
    BookRecord record;

    int titleLen, authorLen, isbnLen;

    //Title
    if (fread(&titleLen, sizeof(int), 1, file) != 1)//read tile length
    {
        record.bookTitle = NULL;
        return record;
    }
    record.bookTitle = (char *)malloc(titleLen + 1);

    if (fread(record.bookTitle, sizeof(char), titleLen, file) != titleLen)// read title
    {
        free(record.bookTitle);
        record.bookTitle = NULL;
        return record;
    }
    record.bookTitle[titleLen] = '\0';

    //Author
    if (fread(&authorLen, sizeof(int), 1, file) != 1)//read author length
    {
        free(record.bookTitle);
        record.bookTitle = NULL;
        return record;
    }
    record.authorName = (char *)malloc(authorLen + 1);
    if (fread(record.authorName, sizeof(char), authorLen, file) != authorLen)//read author
    {
        free(record.bookTitle);
        free(record.authorName);
        record.bookTitle = NULL;
        record.authorName = NULL;
        return record;
    }
    record.authorName[authorLen] = '\0';

    //ISBN
    if (fread(&isbnLen, sizeof(int), 1, file) != 1)//read ISBN length
    {
        free(record.bookTitle);
        free(record.authorName);
        record.bookTitle = NULL;
        record.authorName = NULL;
        return record;
    }
    record.isbn = (char *)malloc(isbnLen + 1);
    if (fread(record.isbn, sizeof(char), isbnLen, file) != isbnLen)//read ISBN
    {
        free(record.bookTitle);
        free(record.authorName);
        free(record.isbn);
        record.bookTitle = NULL;
        record.authorName = NULL;
        record.isbn = NULL;
        return record;
    }
    record.isbn[isbnLen] = '\0';


    return record;
}
void insertRecord(FILE *file)
{
    BookRecord record;

    printf("Enter Book Title: ");
    readInput(&record.bookTitle);
    printf("Enter Author Name: ");
    readInput(&record.authorName);
    printf("Enter ISBN: ");
    readInput(&record.isbn);

    fseek(file,0,SEEK_END);//new additions will be at the end of the file because there is no deletion.
    writeRecord(file, record);

    freeRecord(record);
}
void printFile(FILE *file)
{
    fseek(file, 0, SEEK_SET);
    printf("\n%-20s | %-20s | %-15s\n", "Book Title", "Author Name", "ISBN");
    printf("--------------------------------------------------------\n");
    BookRecord record;
    while (1)
    {
        record = readRecord(file);
        if (record.bookTitle == NULL)
        {
            break;
        }

        printf("%-20s | %-20s | %-15s\n", record.bookTitle, record.authorName, record.isbn);
        freeRecord(record);
    }
}

void findRecord(FILE *file)
{
    char *searchISBN;
    printf("Enter ISBN to search: ");
    readInput(&searchISBN);

    fseek(file, 0, SEEK_SET);
    BookRecord record;

    while (1)
    {
        record = readRecord(file);

        if (record.bookTitle == NULL)
        {
            break;
        }

        if (strcmp(record.isbn, searchISBN) == 0)
        {
            printf("Record Found:\n");
            printf("\tBook Title: %s\n", record.bookTitle);
            printf("\tAuthor Name: %s\n", record.authorName);
            printf("\tISBN: %s\n", record.isbn);
            freeRecord(record);
            free(searchISBN);
            return;
        }
        freeRecord(record);
    }
    printf("Record with ISBN '%s' not found.\n", searchISBN);
    free(searchISBN);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r+b");
    if (!file)
    {
        file = fopen(filename, "w+b");
        if (!file)
        {
            perror("Error opening or creating file");
            return EXIT_FAILURE;
        }
    }

    int choice;
    do
    {
        printf("\nLibrary Catalog Menu:\n");
        printf("1. Insert Record\n");
        printf("2. Print File\n");
        printf("3. Find Record\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        switch (choice)
        {
        case 1:
            insertRecord(file);
            break;
        case 2:
            printFile(file);
            break;
        case 3:
            findRecord(file);
            break;
        case 0:
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }

    } while (choice != 0);
    fclose(file);
    return 0;
}