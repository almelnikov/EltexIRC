#ifndef ELTEXIRC_SERVER_USERS_H_
#define ELTEXIRC_SERVER_USERS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread_info.h"
#include "irc_limits.h"
#include "ircstr.h"

#define IRC_USERS_MAX 64
#define IRC_CHANUSERS_MAX 32
#define IRC_CHANNEL_MAX 32

#define IRC_USERERR_CANTADD 1
#define IRC_USERERR_NOTFOUND 2
#define IRC_USERERR_EXIST 3
#define IRC_USERERR_NICK 4
#define IRC_USERERR_CHAN 5

struct IRCUser {
  struct Client *thr_info;
  char nick[IRC_NICK_BUF_SIZE];
};

struct IRCAllUsers {
  pthread_mutex_t lock;
  struct IRCUser users[IRC_USERS_MAX];
};

struct IRCChannel {
  int cnt;
  char name[IRC_CHAN_BUF_SIZE];
  struct IRCUser *users[IRC_CHANUSERS_MAX];
};

struct IRCAllChannels {
  pthread_mutex_t lock;
  struct IRCChannel channels[IRC_CHANNEL_MAX];
};

struct NamesList {
  int cnt;
  char **names;
};

void UsersInit(struct IRCAllUsers *users);
void ChannelsInit(struct IRCAllChannels *channels);
struct IRCUser *GetUserPtr(struct IRCAllUsers *allusers, const char *nick);
int RenameUser(struct IRCAllUsers *allusers, const char *oldnick,
               const char *newnick);
int DelUser(struct IRCAllUsers *allusers, const char *nick);
int AddUser(struct IRCAllUsers *allusers, const char *nick,
            struct Client *thr);
int AddUserToChannel(struct IRCAllChannels *channels,
                     struct IRCAllUsers *allusers,
                     const char *channame, const char *nick);
int RemoveUserFromChannel(struct IRCAllChannels *channels,
                          struct IRCAllUsers *allusers,
                          const char *channame, const char *nick);
struct IRCChannel *GetChannelPtr(struct IRCAllChannels *channels,
                                 const char *channame);
struct IRCUser **FindUserOnChan(struct IRCChannel *chan, const char *nick);
struct NamesList GetChannelsList(struct IRCAllChannels *channels);
int GetUsersOnChannel(struct IRCAllChannels *channels,
                      struct IRCAllUsers *allusers, const char *channame,
                      struct NamesList *users_list);
void FreeNamesList(struct NamesList *list);

#endif  // ELTEXIRC_SERVER_USERS_H_
