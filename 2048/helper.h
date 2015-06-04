#include <stdbool.h>

#define FIELD_SIZE (4)
#define SHM_NAME ("/OSUE")

#define SEM_1 ("/OSUE_SEM1")
#define SEM_2 ("/OSUE_SEM2")

#define PERMISSION (0600)

int parse_int(char *, int *);
void *create_shared_memory(size_t, char *, int oflag); 
void bail_out(char *, int, const char *, ...);

enum game_state {
    ST_WON,
    ST_LOST,
    ST_NOSUCHGAME
};

enum game_command {
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
    int game_id; // not sure if necessary
    enum game_state state; 
    enum game_command command;
    int field[FIELD_SIZE*FIELD_SIZE];
};
