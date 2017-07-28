#ifndef _CLIENT_H_
#define _CLIENT_H_


#include <unistd.h>     // close read write
#include <string.h>     // memset
#include <errno.h>      // perror
#include <stdlib.h>     // exit
#include <sys/stat.h>   // stat
#include <dirent.h>     // readdir DIR
#include <netdb.h>      // sockaddr_in
#include <sys/wait.h>   // wait


#include "hash.h"       // hash()
#include "ftree.h"      // request stuct


#endif
