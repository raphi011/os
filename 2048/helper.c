#include <stdlib.h>

/** parse_int
 * @brief Parses an int value
 * @params str The char * that will be parsed
 * @params ptr The int * where the value will be stored
 * @return 1 if str was an int, else 0
 */
int parse_int(char * str, int * ptr) {
    char * endptr;
    
    *ptr = strtol(str, &endptr, 10);
    
    // todo, function doesn't check all errors
    return (*endptr == '\0'); 
}

enum {
    ST_WON,
    ST_LOST,
    ST_NOSUCHGAME
};

enum {
    CMD_LEFT,
    CMD_RIGHT,
    CMD_UP,
    CMD_DOWN,
    CMD_QUIT
};



