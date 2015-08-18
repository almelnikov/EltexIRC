#ifndef ELTEXIRC_SERVER_USERS_H_
#define ELTEXIRC_SERVER_USERS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread_info.h"

#define IRC_NICK_MAX_LENGTH 63
#define IRC_NICK_BUF_SIZE (IRC_NICK_MAX_LENGTH + 1)
#define IRC_USERS_MAX 64
#define IRC_CHANUSERS_MAX 32
#define IRC_CHANNAME_MAX_LENGTH 63
#define IRC_CHAN_BUF_SIZE (IRC_CHANNAME_MAX_LENGTH + 1)
#define IRC_CHANNEL_MAX 32

#define IRC_USERERR_CANTADD 1
#define IRC_USERERR_NOTFOUND 2
#define IRC_USERERR_EXIST 3

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
int RemoveUserFromChannel(struct IRCAllChannels *channels, const char *channame,
                          const char *nick);
struct IRCChannel *GetChannelPtr(struct IRCAllChannels *channels,
                                 const char *channame);
struct IRCUser **FindUserOnChan(struct IRCChannel *chan, const char *nick);
struct NamesList GetChannelsList(struct IRCAllChannels *channels);
void FreeNamesList(struct NamesList *list);
int IsValidNick(const char *nick);
int IsValidChannel(const char *chan);

#endif  // ELTEXIRC_SERVER_USERS_H_
