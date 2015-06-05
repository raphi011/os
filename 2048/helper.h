#include <stdbool.h>

#define FIELD_SIZE (4)
#define SHM_NAME ("/OSUE")

#define SEM_1 ("/OSUE_SEM1")
#define SEM_2 ("/OSUE_SEM2")

#define SEM_GAME ("/OSUE_GAME")

#define PERMISSION (0600)

extern char * modulname; 

int parse_int(char *, int *);
void *create_shared_memory(size_t, char *, int); 
void bail_out(int, const char *, ...);
char *get_game_sem(int, int);

enum game_state {
    ST_RUNNING,
    ST_WON,
    ST_LOST,
    ST_NOSUCHGAME
};

enum game_command {
    CMD_NONE,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_UP,
    CMD_DOWN,
    CMD_QUIT
};

struct connect {
    bool new_game;
    int game_id; 
};


struct game_data {
    enum game_state state; 
    enum game_command command;
    int field[FIELD_SIZE*FIELD_SIZE];
};
