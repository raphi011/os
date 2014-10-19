/**
 * @file stegit.c
 * @author Raphael Gruber <raphi011@gmail.com>
 * @date 17.10.2014
 * @brief stegit program module.
 * @details Test
 **/

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define BUFFER_LENGTH (300)

static char* modulname;
static FILE* out;

static const char* words[] = {
	"palilicium",
	"polyxenus",
	"overbrave",
	"isotron",
	"societal",
	"backdown",
	"pekinese",
	"effect",
	"superfinical",
	"unabetted",
	"goyen",
	"nondebtor",
	"equiprobabilism",
	"henceforth",
	"axiomatically",
	"grogshop",
	"unweighty",
	"stylelessness",
	"prosaically",
	"maiolica",
	"climactic",
	"axilemma",
	"cliffy",
	"ulcerating",
	"interjectural",
	"rustred",
	"unsimulated",
	"jerrycan"
};

#define EXIT_ERR() { usage(); closeOut(); return(1); }

#define CHECK_PRINTF(returnValue) {  if (returnValue < 0) { \
(void)fprintf(stderr, "%s: writing to out failed",modulname); exit(2); } }

/*
 Name: closeOut
 Desc: closes the out file
 Args: - 
 Returns: -
*/
void closeOut(void) {
	if (out != NULL) 
		(void)fclose(out);
}

/*
 Name: usage
 Desc: prints the programs usage
 Args: -
 Returns: -
*/
void usage(void) {
	(void)fprintf(stderr, 	"Usage: %s -f|-h [-o <filename>]\n\n" \
							"-f\t\tfind mode\n" \
							"-h\t\thide mode\n" \
							"[-o <filename>]\toutput filename\n", modulname);
}

/*
 Name: nextDot
 Desc: creates a random number between 5-14
 Args: -
 Returns: the amount of words after the next random dot
*/
int nextDot(void) {
	return (rand() % 10) + 5;
}

/*
 Name: wordToChar
 Desc: looks up index in words array
 Args: word : the word to look for
 Returns: index in words array
*/
int wordToChar(char * word) {

	for (int i = 0; i < (sizeof(words) / sizeof(words[0])); i++) {
		if (strcmp(words[i], word) == 0)
			return i;
	}

	return -1;
}

/*
 Name: getInputLine
 Desc: reads 300 chars from stdin
 Args: -
 Returns: user input string (malloced)
*/
char* getInputLine(void) {

	char *line = malloc(BUFFER_LENGTH);

	if (line == NULL)
		return NULL;

	(void)fgets(line, BUFFER_LENGTH, stdin);

	if (ferror(stdin)) {
		(void)fprintf(stderr, "%s: reading from stdin failed",modulname); 
		exit(2);
	}

    return line;
}


/*
 Name: hide
 Desc: takes a string and hides the contents in a special 'encoding'
 Args:	in : input string
 		out : output file
 Returns: - 
*/
void hide(const char* in, FILE* out) {

	srand(time(NULL));
	
	int dot = nextDot();
	int ret;

	for (int i = 0; in[i] != '\0'; i++) {

		int c = in[i];

		if (c >= 'a' && c <= 'z') 
			c = c - 'a';
		else if (c == '.' ) // '.' char
			c = 26;
		else if (c == ' ') // ' ' char
			c = 27;
		else // ignore everything else
			continue;

		if (dot-- == 0) {
			ret = fprintf(out, "%s. ", words[c]);
			dot = nextDot();
		}
		else  {
			ret = fprintf(out, "%s ", words[c]);
		}

		CHECK_PRINTF(ret)
	}
}

/* 
 Name: find
 Desc: converts the hidden message to a readable string
 Args:	in : input string
		out : output file ( e.g. stdout )
 Returns: -
*/
void find(char* in, FILE * out) {
	
	char* word;
	char* ptr;

	for (word = strtok_r(in, " .", &ptr);
         word;
         word = strtok_r(NULL, " .", &ptr)) { // split line at ' ' and '.' characters

		int index = wordToChar(word);

		if (index == -1)
			continue;

		char c;

		if (index >= 0 && index < 26)
			c = index + 'a';
		else if (index == 26)
			c = '.';
		else if (index == 27)
			c = ' ';	
		else 
			continue;

		CHECK_PRINTF(fprintf(out, "%c", c))
	}
}


int main(int argc, char** argv)
{
	modulname = argv[0];

	char* outFileName = NULL;
	
	int c;

  	opterr = 0;

  	bool fOption = false;
  	bool hOption = false;

	while ((c = getopt (argc, argv, "fho:")) != -1)
		switch (c)
		{
			case 'f':
				fOption = true;
				break;
			case 'h':
				hOption = true;
				break;
			case 'o':
				outFileName = optarg;
				break;
			case '?':
				EXIT_ERR()
			default:
				assert(0);
	}

	// check if either find OR hide option is set
	if (fOption == hOption) {
		EXIT_ERR()
	}

	char* input = getInputLine();

	if (input == NULL) {
		(void)fprintf(stderr, "Failed to reserve space for input");
		return 2;
	}

	out = stdout;

	if (outFileName != NULL) {
		out = fopen(outFileName, "w");
	}

	if (fOption) 
		find(input, out);
	else 
		hide(input, out);
	
	free(input);
	(void)fclose(out);

	return 0;
}	

