#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BOOK_TITLE_LENGTH 20
#define AUTHOR_NAME_LENGTH 20
#define ISBN_LENGTH 14
#define RECORD_LENGTH (BOOK_TITLE_LENGTH + AUTHOR_NAME_LENGTH + ISBN_LENGTH + 1) //+1  for "is deleted"

typedef struct // struct for Books //65byte
{
    char bookTitle[BOOK_TITLE_LENGTH];
    char authorName[AUTHOR_NAME_LENGTH];
    char isbn[ISBN_LENGTH];
    bool isDeleted;
} BookRecord;

typedef struct availListNode // ll stack structure for availList
{
    int offset;
    struct availListNode *next;
} AvailListNode;

AvailListNode *availListHead = NULL;

void readInput(char *input, int size)
{
    int i = 0;
    char c;

    // Read characters until newline, EOF, or buffer full
    while (i < size - 1 && (c = getchar()) != '\n' && c != EOF)
    {
        input[i++] = c;
    }

    // Null-terminate the string if space remain
    input[i] = '\0';

    // Clear input buffer if the input was longer than size
    if (c != '\n' && c != EOF)
    {
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}

void writeRecord(FILE *file, BookRecord record, int offset) // Function to write record at a given offset
{
    fseek(file, offset, SEEK_SET);
    fwrite(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file);   // 20b
    fwrite(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file); // 20b
    fwrite(record.isbn, sizeof(char), ISBN_LENGTH, file);              // 10b
    fwrite(&record.isDeleted, sizeof(bool), 1, file);                  // 1b
}

// Function to read record at a given offset
BookRecord readRecord(FILE *file, int offset)
{
    BookRecord record;
    fseek(file, offset, SEEK_SET);
    fread(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file);
    fread(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file);
    fread(record.isbn, sizeof(char), ISBN_LENGTH, file);
    fread(&record.isDeleted, sizeof(bool), 1, file);
    return record;
}
// Function to add to availList
void addToAvailList(int offset)
{
    AvailListNode *newNode = (AvailListNode *)malloc(sizeof(AvailListNode));
    newNode->offset = offset;
    newNode->next = availListHead;
    availListHead = newNode;
}

// Function to get available slot from availList
int getAvailListSlot()
{
    if (availListHead == NULL)
    {
        return -1;
    }

    int offset = availListHead->offset;
    AvailListNode *temp = availListHead;
    availListHead = availListHead->next;
    free(temp);
    return offset;
}

void insertRecord(FILE *file)
{
    BookRecord record;
    int offset;

    printf("Enter Book Title: ");
    readInput(record.bookTitle, BOOK_TITLE_LENGTH);
    printf("Enter Author Name: ");
    readInput(record.authorName, AUTHOR_NAME_LENGTH);
    printf("Enter ISBN: ");
    readInput(record.isbn, ISBN_LENGTH);
    record.isDeleted = false;

    offset = getAvailListSlot();

    if (offset != -1)
    {
        writeRecord(file, record, offset);
    }
    else // if the file is empty
    {
        fseek(file, 0, SEEK_END);
        offset = ftell(file);
        writeRecord(file, record, offset);
    }
}

void printFile(FILE *file)
{
    fseek(file, 0, SEEK_SET);
    printf("\n%-*s | %-*s | %-*s | %-5s\n",
           -BOOK_TITLE_LENGTH, "Book Title",
           -AUTHOR_NAME_LENGTH, "Author Name",
           -ISBN_LENGTH, "ISBN", "Deleted");
    printf("-----------------------------------------------------------------\n");

    BookRecord record;
    long offset = 0;
    while (fread(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file) == BOOK_TITLE_LENGTH)
    {

        fread(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file);
        fread(record.isbn, sizeof(char), ISBN_LENGTH, file);
        fread(&record.isDeleted, sizeof(bool), 1, file);
        printf("%-*s | %-*s | %-*s | %-5s\n",
               -BOOK_TITLE_LENGTH, record.bookTitle,
               -AUTHOR_NAME_LENGTH, record.authorName,
               -ISBN_LENGTH, record.isbn, record.isDeleted ? "true" : "false");
    }
}

void findRecord(FILE *file)
{
    char isbn[ISBN_LENGTH];
    printf("Enter ISBN to search: ");
    readInput(isbn, ISBN_LENGTH);

    int offset = BOOK_TITLE_LENGTH + AUTHOR_NAME_LENGTH;
    int isbnOffset = offset;
    int i = 0;
    fseek(file, isbnOffset, SEEK_SET);

    BookRecord record;
    while (fread(record.isbn, sizeof(char), ISBN_LENGTH, file)==ISBN_LENGTH) // fread(record.isbn, sizeof(char), ISBN_LENGTH, file) == ISBN_LENGTH
    {
        fread(&record.isDeleted, sizeof(bool), 1, file);

        if (!record.isDeleted && strcmp(isbn, record.isbn) == 0)
        {
            fseek(file, isbnOffset-offset, SEEK_SET);
            fread(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file);
            fread(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file);

            printf("Record Found:\n");
            printf("\tBook Title\t: %s\n", record.bookTitle);
            printf("\tAuthor Name\t: %s\n", record.authorName);
            printf("\tISBN\t\t: %s\n", record.isbn);
            return;
        }

        isbnOffset = isbnOffset + RECORD_LENGTH;
        fseek(file, isbnOffset, SEEK_SET);
    }

    printf("Record with ISBN '%s' not found.\n", isbn);
}

void deleteRecord(FILE *file)
{
    char isbn[ISBN_LENGTH];
    printf("Enter ISBN of record to delete: ");
    readInput(isbn, ISBN_LENGTH);

    fseek(file, 0, SEEK_SET);
    BookRecord record;
    int offset = 0;
    while (fread(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file) == BOOK_TITLE_LENGTH)
    {
        fread(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file);
        fread(record.isbn, sizeof(char), ISBN_LENGTH, file);
        fread(&record.isDeleted, sizeof(bool), 1, file);

        if (!record.isDeleted && strcmp(isbn, record.isbn) == 0)
        {
            fseek(file, -sizeof(bool), SEEK_CUR);
            bool deleted = true;
            fwrite(&deleted, sizeof(bool), 1, file);

            addToAvailList(offset);
            printf("Record with ISBN '%s' marked as deleted.\n", isbn);
            return;
        }
        offset += RECORD_LENGTH + sizeof(bool);
    }
    printf("Record with ISBN '%s' not found.\n", isbn);
}

// Function to compact the file (remove deleted records)
void compactFile(FILE *file, const char *filename)
{
    FILE *tempFile = fopen("temp.dat", "wb"); // Create a temporary file
    if (tempFile == NULL)
    {
        perror("Error creating temporary file");
        return;
    }
    fseek(file, 0, SEEK_SET);
    BookRecord record;

    while (fread(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, file) == BOOK_TITLE_LENGTH)
    {

        fread(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, file);
        fread(record.isbn, sizeof(char), ISBN_LENGTH, file);
        fread(&record.isDeleted, sizeof(bool), 1, file);
        if (!record.isDeleted)
        {
            fwrite(record.bookTitle, sizeof(char), BOOK_TITLE_LENGTH, tempFile);
            fwrite(record.authorName, sizeof(char), AUTHOR_NAME_LENGTH, tempFile);
            fwrite(record.isbn, sizeof(char), ISBN_LENGTH, tempFile);
            fwrite(&record.isDeleted, sizeof(bool), 1, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);
    remove(filename);
    rename("temp.dat", filename);

    file = fopen(filename, "r+");
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
        printf("4. Delete Record\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;

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
        case 4:
            deleteRecord(file);
            break;
        case 0:
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    compactFile(file, filename);
    fclose(file);
    return 0;
}