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
#include "irc_msgread.h"


#define PREAMBLE_SIZE 200 //исправить
#define HOSTNAME_MAX 2 * IRC_NICK_MAX_LENGTH + 2
#define IRC_MSG_SIZE 513

union RegistrationFlags
{
	struct RegFlags
	{
		unsigned user:1;
		unsigned nick:1;
    unsigned connect:1;
		unsigned fail:1;
  } flags;
	unsigned clear;
};

struct ThreadChanNode
{
  char chan[20];
  struct ThreadChanNode *next;
};

struct ThreadChanList
{
  struct ThreadChanNode *head;
  int size;
};

union RegistrationFlags registered;
struct IRCAllUsers all_users;
struct IRCAllChannels all_chan;

struct ThreadChanNode *CreateThrNode(char *data);
struct ThreadChanNode *ThrListAddFront(struct ThreadChanList *list,
                                char *chan);
struct ThreadChanNode *DeleteThrNode(struct ThreadChanList *list,
                                  char *chan);
void FreeThreadList(struct ThreadChanList *list);

#endif // ELTEXIRC_SERVER_CONNECT_H
