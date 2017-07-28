#ifndef _SERVER_H_
#define _SERVER_H_

#include <unistd.h>     // close read write
#include <string.h>     // memset
#include <errno.h>      // perror
#include <stdlib.h>     // exit
#include <sys/stat.h>   // stat
#include <netdb.h>      // sockaddr_in

#include "hash.h"       // hash()
#include "ftree.h"      // request stuct

/**
 * A client Link List node
 * fd				file descriptor of the file
 * current_state	the current state of the client
 * file				the file to be synced
 * client_req		the client request
 * next				the next client node
 */
struct client {
    int fd;
    int current_state;
    FILE *file;
    struct request client_req;
    struct client *next;
};

#endif // _SERVER_H_
