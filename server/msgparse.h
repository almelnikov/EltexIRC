#ifndef ELTEXIRC_SERVER_MSGPARSE_H_
#define ELTEXIRC_SERVER_MSGPARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum IRCCommands {
  IRCCMD_UNKNOWN,
  IRCCMD_PASS,
  IRCCMD_USER,
  IRCCMD_NICK,
  IRCCMD_PRIVMSG,
  IRCCMD_JOIN,
  IRCCMD_PART,
  IRCCMD_QUIT,
  IRCCMD_LIST
};

struct ParsedMsg {
  char *optional;
  enum IRCCommands cmd;
  int cnt;
  char **params;
};

int FormParsedMsg(const char *str, struct ParsedMsg *msg);
void FreeParsedMsg(struct ParsedMsg *msg);

#endif  // ELTEXIRC_SERVER_MSGPARSE_H_
