#ifndef ELTEXIRC_SERVER_PRIVMSGSEND_H_
#define ELTEXIRC_SERVER_PRIVMSGSEND_H_

#include "users.h"
#include "thread_info.h"

int SendMsgToUser(struct IRCAllUsers *clients, const char *nick,
                  const char *msg);
int SendMsgToChannel(struct IRCAllChannels *channels,
                     struct IRCAllUsers *clients, const char *channame,
                     const char *nick, const char *msg);

#endif  // ELTEXIRC_SERVER_PRIVMSGSEND_H_