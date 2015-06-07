#include <stdbool.h>

#define FIELD_SIZE (4)

#define SHM_CON ("/OSUE_CON")
#define SHM_GAME ("/OSUE_GAME")

#define SEM_CON_1 ("/OSUE_CON1")
#define SEM_CON_2 ("/OSUE_CON2")

#define SEM_GAME ("/OSUE_GAME")

#define PERMISSION (0600)

extern char * modulname; 

int get_index(int, int);
char *get_game_shm(int);
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
