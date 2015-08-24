#ifndef ELTEXIRC_SERVER_MSGPARSE_H_
#define ELTEXIRC_SERVER_MSGPARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ERR_NONICKNAMEGIVEN 431
#define ERR_NICKNAMEINUSE 436
#define ERR_ERRONEUSNICKNAME 433
#define ERR_NEEDMOREPARAMS 461
#define ERR_CHANNELISFULL 471

enum IRCCommands {
  IRCCMD_UNKNOWN,
  IRCCMD_PASS,
  IRCCMD_USER,
  IRCCMD_NICK,
  IRCCMD_PRIVMSG,
  IRCCMD_JOIN,
  IRCCMD_PART,
  IRCCMD_QUIT,
  IRCCMD_LIST,
  IRCCMD_PING,
  IRCCMD_PONG,
  IRCCMD_NUMERIC
};

struct ParsedMsg {
  char *optional;
  enum IRCCommands cmd;
  int cnt;
  char **params;
  int numeric;
};

int FormParsedMsg(const char *str, struct ParsedMsg *msg);
void FreeParsedMsg(struct ParsedMsg *msg);
char *GetNickFromHost(struct ParsedMsg *msg);

#endif  // ELTEXIRC_SERVER_MSGPARSE_H_
