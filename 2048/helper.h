/** 
 * @file helper.h 
 * @author Raphael Gruber (0828630)
 * @brief Shared functions for 2048 client / server 
 * @date 07.06.2015
 */

#include <stdbool.h>

/**
 * size of a field dimension
 */
#define FIELD_SIZE (4)

/**
 * name of shared memory
 */
#define SHM_CON ("/OSUE_CON")
#define SHM_GAME ("/OSUE_GAME")

/**
 * name of connection semaphore
 */
#define SEM_CON_1 ("/OSUE_CON1")
#define SEM_CON_2 ("/OSUE_CON2")

/**
 * name of game semaphore
 */
#define SEM_GAME ("/OSUE_GAME")

/**
 * semaphore permissions
 */
#define PERMISSION (0600)

extern char * modulname; 

/** sem_post_sec
 * @brief does sem_post on a semaphore and exists if an error occurs
 * @params the semaphore
 */
void sem_post_sec(sem_t *sem);

/** sem_wait_sec
 * @brief does sem_wait on a semaphore and exists if an error occurs
 * @params sem the semaphore
 */
void sem_wait_sec(sem_t *sem);

/** get_index
 * @brief calculates the (x,y) index for a one dimensional array
 * @params x the x value
 * @params y the y value
 * @return the index
 */
int get_index(int, int);

/**  get_game_shm
 * @brief gets the name for the shared memory with a specific id
 * @params game_id  the name of the shared memory
 */
char *get_game_shm(int);

/** parse_int
 * @brief Parses an int value
 * @params str The char * that will be parsed
 * @params ptr The int * where the value will be stored
 * @return 1 if str was an int, else 0
 */
long parse_int(char *, int *);

/**
 * create_shared_memory
 * @brief creates a shared memory mapping
 * @param size size of the shared memory
 * @param name name of the shared memory
 * @param oflag the oflag
 * @return returns a pointer to the allocated shared memory
 */
void *create_shared_memory(size_t, char *, int); 

/**
 * bail_out
 * @brief Stops the program gracefully, freeing resources and printing an
 * error message to stderr
 * @param eval The return code of the program
 * @param fmt The error message
 * @param ... message format parameters
 */
void bail_out(int, const char *, ...);

/**  get_game_sem
 * @brief gets the name for the semaphore with a specific id and sem number
 * @params game_id the name of the shared memory
 * @params sem_num the sem number
 */
char *get_game_sem(int, int);

/**
 * available game states
 */
enum game_state {
    ST_RUNNING,
    ST_WON,
    ST_LOST,
    ST_NOSUCHGAME
};

/**
 * available user commands
 */
enum game_command {
    CMD_NONE,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_UP,
    CMD_DOWN,
    CMD_QUIT
};

/**
 * contains data for establishing a connection
 */
struct connect {
    bool new_game;
    int game_id; 
};


/**
 * contains the game data
 */
struct game_data {
    enum game_state state; 
    enum game_command command;
    int field[FIELD_SIZE*FIELD_SIZE];
};
