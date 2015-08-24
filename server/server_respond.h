#ifndef ELTEXIRC_SERVER_USERS_LIST_H_
#define ELTEXIRC_SERVER_USERS_LIST_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "irc_limits.h"
#include "users.h"
#include "privmsgsend.h"

#define IRCERR_SCL_LIST 1
#define IRCERR_SCL_WRITE 2

int SendConnectMsg(struct IRCAllUsers *allusers, const char *host,
                   const char *nick);
int SendAllChannelsList(int sock, struct IRCAllChannels *channels,
                        struct IRCAllUsers *allusers, const char *host,
                        const char *nick);
int SendChannelList(int sock, struct IRCAllChannels *channels,
                    struct IRCAllUsers *allusers, const char *channame,
                    const char *host, const char *nick);

#endif  // ELTEXIRC_SERVER_USERS_LIST_H_