#ifndef ELTEXIRC_LIB_IRC_MSGREAD_H_
#define ELTEXIRC_LIB_IRC_MSGREAD_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "irc_limits.h"

int IRCMsgRead(int sockfd, char *raw_msg);

#endif  // ELTEXIRC_LIB_IRC_MSGREAD_H_