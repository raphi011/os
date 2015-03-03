#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * modulename; 

#define MAX_CHARS (100)
#define MIN(a,b) (a>b?b:a)

int compareRows(char * row1, char * row2) {
    int length = MIN(strlen(row1),strlen(row2)) - 1; // length minus '\0'
    //printf("%d \n",length);
    int errors = 0;

    for (int i=0; 
         i < length; 
         i++) {
        if (*row1++ != *row2++) {
            errors++;
        }
    }
    return (errors);
}


int main(int argc, char ** argv) {
    modulename = argv[0];
    
    char * path1 = argv[1];
    char * path2 = argv[2];
    
    FILE * file1 = fopen(path1, "r");
    FILE * file2 = fopen(path2, "r");

    char * line1 = malloc(MAX_CHARS);
    char * line2 = malloc(MAX_CHARS);

    int curRow = 1;

    while (fgets(line1, MAX_CHARS, file1) && fgets(line2, MAX_CHARS, file2)) {
        int errors = compareRows(line1,line2);       
        
        if (errors) {
            printf("Zeile: %d Zeichen: %d\n", curRow, errors);
        }
        
        curRow++;
    }
}

