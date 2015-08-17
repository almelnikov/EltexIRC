#ifndef ELTEXIRC_SERVER_CONNECT_H
#define ELTEXIRC_SERVER_CONNECT_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include "thread_info.h"

#define IRC_MSG_SIZE 513
#define IRC_MAX_CLIENT 20

struct Client client_pool[IRC_MAX_CLIENT];

int search_available();
int IRC_Msg_Read(int sockfd, char *buf);

#endif
