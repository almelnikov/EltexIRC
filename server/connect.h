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
#include "users.h"

#define IRC_MSG_SIZE 513

union RegistrationFlags
{
	struct flags
	{
		unsigned user:1;
		unsigned nick:1;
		unsigned fail:1;
	}
	unsigned clear;
};

union RegistrationFlags registered;
struct IRCAllUsers all_users;
struct IRCAllChannels all_chan;

int IRCMsgRead(int sockfd, char *buf);
void Release(struct IRCUser *user);

#endif
