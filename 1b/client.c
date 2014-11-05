#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *serv;
    int sockfd, error;
   
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo("localhost", "1280", &hints, &serv);
    
    if (error != 0) {
        printf("getaddrinfo error");
        // do something
    }
    
    sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);    
    
    if (sockfd < 0) {
        printf("socket error");
        // error    
    } 

    /* Connect the socket to the address */
    if (connect(sockfd, serv->ai_addr, serv->ai_addrlen) != 0) {
        printf("connect error");
        // error
    }

    uint8_t color = 1;
    
    if (write(sockfd, &color, sizeof(color)) < 0) {
        printf("write error");
    }
    freeaddrinfo(serv);
}
