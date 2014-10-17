/**
 * @file stegit.c
 * @author Raphael Gruber <raphi011@gmail.com>
 * @date 17.10.2014
 * @brief stegit program module.
 * @details Test
 **/

#include "stegit.h"

#define BUFFER 300
#define WORD_COUNT 28
#define ASCII_LOWERCASE_A 97

char* modulname;
FILE * out;

char* words[] = {
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



int main(int argc, char **argv)
{
	modulname = argv[0];

	char *outFileName = NULL;
	
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

	if (fOption && hOption) 
		EXIT_ERR()

	char * input = getInputLine();

	out = stdin;

	if (outFileName != NULL) 
		out = fopen(outFileName, "w");

	if (fOption) 
		find(input, out);
	else 
		hide(input, out);
	
	free(input);
	(void)fclose(out);

	return 0;
}	

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
	(void)fprintf(stderr, "Usage: %s -f|-h [-o <filename>]\n\n-f\t\tfind mode\n-h\t\thide mode\n[-o <filename>]\toutput filename\n", modulname);
}

/*
 Name: nextDot
 Desc: creates a random number between 5-15
 Args: -
 Returns: the amount of words after the next random dot
*/
int nextDot(void) {
	return (rand() % 10) + 5;
}

/*
 Name: hide
 Desc: takes a string and hides the contents in a special 'encoding'
 Args:	in : input string
 		out : output file
 Returns: - 
*/
void hide(char* in, FILE * out) {

	srand(time(NULL));
	
	int dot = nextDot(), ret;

	for (int i = 0; in[i] != '\0'; i++) {

		int c = in[i] - ASCII_LOWERCASE_A;

		char *word = NULL;

		if (c >= 0 && c < 26) 
			word = words[c];
		else if (c == -51) // '.' char
			word = words[26];
		else if (c == -65) // ' ' char
			word = words[27];

		if (word == NULL) 
			continue;

		if (dot-- == 0) {
			ret = fprintf(out, "%s. ", word);
			dot = nextDot();
		}
		else  
			ret = fprintf(out, "%s ", word);

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
	
	char *tok;
	char *ptr = in;

	int ret;

	while ((tok = strtok(ptr, " .")) != NULL) { // split line at ' ' and '.' characters

		int index = wordToChar(tok);

		if (index == -1)
			continue;

		char c;

		if (index >= 0 && index < 26)
			c = index + ASCII_LOWERCASE_A;
		else if (index == 26)
			c = '.';
		else if (index == 27)
			c = ' ';	
		else 
			continue;

		ret = fprintf(out, "%c", c);
		
		CHECK_PRINTF(ret)

		ptr = NULL;
	}
}

/*
 Name: wordToChar
 Desc: looks up index in words array
 Args: word : the word to look for
 Returns: index in words array
*/
int wordToChar(char * word) {

	for (int i = 0; i < WORD_COUNT; i++) {
		if (strcmp(words[i], word) == 0)
			return i;
	}

	return -1;
}

/*
 Name: getInputLine
 Desc: reads 300 chars from stdin
 Args: -
 Returns: user input string
*/
char* getInputLine(void) {

	char *line = malloc(BUFFER);

	if (line == NULL)
		return NULL;

	(void)fgets(line, BUFFER, stdin);


    return line;
}
