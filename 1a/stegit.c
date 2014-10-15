#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char *file = NULL;
	int index;
	int c;

  	opterr = 0;


	while ((c = getopt (argc, argv, "fho:")) != -1)
		switch (c)
		{
			case 'f':
				printf("option -f\n");
				break;
			case 'h':
				printf("option -h\n");
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
				abort ();
	}

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	return 0;
}
