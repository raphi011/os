/**
 * @file stegit.c
 * @author Raphael Gruber <raphi011@gmail.com>
 * @date 15.10.2014
 * @brief stegit program module.
 * @details Test
 **/

#include "stegit.h"

 #define BUFFER 100

int main(int argc, char **argv)
{
	char *file = NULL;
	//int index;
	int c;

  	opterr = 0;

  	bool find = false;
  	bool hide = false;

	while ((c = getopt (argc, argv, "fho:")) != -1)
		switch (c)
		{
			case 'f':
				find = true;
				break;
			case 'h':
				hide = true;
				break;
			case 'o':
				file = optarg;
				printf("%s\n", file);
				break;
			case '?':
				if (optopt == 'o')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				assert(0);
	}

	if (find && hide) {
		fprintf(stderr, "Use either -h or -f option");
		return 1;
	}

	char * input = getInputLine();

	printf("You wrote this: %s", input);

	free(input);

	return 0;
}

char * getInputLine(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}


