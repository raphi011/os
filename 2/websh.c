#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define MAX_ROW_LENGTH (50)

static char *modulname;

static void usage() {
	(void)fprintf(stderr, "Usage: %s [-e] [-h] [-s WORD:TAG]", modulname);
	exit(EXIT_FAILURE);
}


static void spawnchild(bool eOption, bool hOption, bool sOption, char* searchReplaceParam) {


}



int main(int argc, char *argv[]) {

	int c;
	char* searchReplaceParam;
	bool eOption, hOption, sOption;

	if (argc > 0) {
		modulname = argv[0];
	}


	while ((c = getopt(argc, argv, "ehs:")) != -1) {
		switch (c) {
			case 'e':
				eOption = true;
				break;
			case 'h':
				hOption = true;
				break;
			case 's': 
				sOption = true;
				searchReplaceParam = optarg;
				break;
			case '?':
				usage();
			default:
				assert(0);

		}
	}

	spawnchild(eOption,hOption,sOption, searchReplaceParam);




	return EXIT_SUCCESS;
}
