/**
 * mastermind client
 * @file client.c
 * @author Raphael Gruber <raphi011@gmail.com>
 * @brief mastermind client implementation
 * @details communicates with a mastermind server and trys to guess
 * the correct order of colors 
 * @date 15.04.2015
 **/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>

#define SLOTS (5)
#define SHIFT_WIDTH (3)
#define SHIFT_WIDTH (3)

#define READ_BYTES (1)
#define WRITE_BYTES (2)

#define PARITY_ERR_BIT (6)
#define GAME_LOST_ERR_BIT (7)

#define EXIT_MULTIPLE_ERRORS (4)

#define EXIT_PARITY_ERROR (2)
#define EXIT_GAME_LOST (3)
#define EXIT_MULTIPLE_ERRORS (4)

/* Name of the program */
static const char *progname = "client";

/**
 * @brief terminate program on program error
 * @param eval exit code
 * @param fmt format string
 */
static void bail_out(int eval, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));        
    }
    (void) fprintf(stderr, "\n");

    exit(eval);
}

/**
 * @brief Read message from socket
 * 
 * This code *illustrates* one way to deal with partial reads 
 *
 * @param sockfd_con Socket to read from
 * @param buffer Buffer where read data is stored
 * @param n Size to read
 * @return Pointer to buffer on success, else NULL
 */
static uint8_t *read_from_server(int fd, uint8_t *buffer, size_t n)
{
    /* loop, as packet can arrive in several partial reads */
    size_t bytes_recv = 0;
    do {
        ssize_t r;
        r = read(fd, buffer + bytes_recv, n - bytes_recv);
        if (r <= 0) {
            return NULL;
        }
        bytes_recv += r;
    } while (bytes_recv < n);

    if (bytes_recv < n) {
        return NULL;
    }
    return buffer;
}

/**
 * @brief create a new guess
 * 
 * out of lack of time I couldn't create a guessing system.
 * it always guesses 'wwrgb'
 *
 * @param message array to store the message
 */
static void next_guess(uint8_t *message) {
    enum { beige, darkblue, green, orange, red, black, violet, white };
    uint16_t request = 0;
    request <<= SHIFT_WIDTH;  
    request |= beige;
    request <<= SHIFT_WIDTH;  
    request |= green;
    request <<= SHIFT_WIDTH;  
    request |= red;
    request <<= SHIFT_WIDTH;  
    request |= white;
    request <<= SHIFT_WIDTH;  
    request |= white;

    message[0] = request; 
    message[1] = request >> 8; 
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_PARITY_ERROR in case of an parity
 * error, EXIT_GAME_LOST in case client needed to many guesses,
 * EXIT_MULTIPLE_ERRORS in case multiple errors occured in one round
 */
int main(int argc, char *argv[])
{
    char *server_arg;
    char *port_arg;
    struct addrinfo hints, *serv;
    int sockfd;
    int error;
    int ret = 0;


    if (argc > 0) {
        progname = argv[0];
    }
    
    if (argc < 3) {
        bail_out(EXIT_FAILURE, "Usage: %s <server-hostname> <server-port>", progname);
    }

    server_arg = argv[1];
    port_arg = argv[2];

     
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(server_arg, port_arg, &hints, &serv);
    
    if (error != 0) {
        bail_out(EXIT_FAILURE, "getaddrinfo error");
    }
    
    sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);    
    
    if (sockfd < 0) {
        bail_out(EXIT_FAILURE, "socket error");
    } 

    /* Connect the socket to the address */
    if (connect(sockfd, serv->ai_addr, serv->ai_addrlen) != 0) {
        bail_out(EXIT_FAILURE, "connect error");
    }

    for (int round = 0; ; round++) {
        static uint8_t buffer;
        bool parity_error;
        bool game_lost;
        bool game_won;

        uint8_t color[2]; 
        
        next_guess(color);
    
        if (write(sockfd, &color, sizeof(color)) < 0) {
            bail_out(EXIT_FAILURE, "write error");
        }

        if (read_from_server(sockfd, &buffer, READ_BYTES) == NULL) {
            bail_out(EXIT_FAILURE, "read_from_server");
        }
        
        /* read status flags and evaluate if we've won */
        parity_error = buffer & (1 << PARITY_ERR_BIT);
        game_lost = buffer & (1 << GAME_LOST_ERR_BIT);
        game_won = (buffer & SLOTS) == SLOTS;

        if (parity_error) { 
            (void)fprintf(stderr, "Parity error");
            ret = EXIT_PARITY_ERROR;
        } 
        if (game_lost) {
            (void)fprintf(stderr, "Game lost"); 
            if (ret == EXIT_PARITY_ERROR) {
                ret = EXIT_MULTIPLE_ERRORS;
            } else {
                ret = EXIT_GAME_LOST; 
            }
        }   

        if (game_won) {
            (void)printf("Won after %d round(s)", round+1);
            break;
        } else if (ret) {
            break;
        }
    } 

    freeaddrinfo(serv);

    return ret;
}
