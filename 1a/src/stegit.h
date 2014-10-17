
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>

char * getInputLine(void);
void find(char* in, FILE * out);
void hide(char* in, FILE * out);
void initiateArray();
int wordToChar(char * word);
int nextDot(void);
void closeOut(void);
void usage(void);
