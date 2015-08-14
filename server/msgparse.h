#ifndef ELTEXIRC_SERVER_MSGPARSE_H_
#define ELTEXIRC_SERVER_MSGPARSE_H_

enum IRCCommands {
  IRCCMD_PASS,
  IRCCMD_USER,
  IRCCMD_PRIVMSG,
  IRCCMD_JOIN,
  IRCCMD_PART,
  IRCCMD_QUIT
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
