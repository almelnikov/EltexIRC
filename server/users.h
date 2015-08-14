#ifndef ELTEXIRC_SERVER_USERS_H_
#define ELTEXIRC_SERVER_USERS_H_

#include <string.h>
#include <pthread.h>
#include "thread_info.h"

#define IRC_NICK_MAX_LENGTH 63
#define IRC_NICK_BUF_SIZE (IRC_NICK_MAX_LENGTH + 1)
#define IRC_USERS_MAX 64
#define IRC_CHANUSERS_MAX 32
#define IRC_CHANNAME_MAX_LENGTH 200
#define IRC_CHAN_BUF_SIZE (IRC_CHANNAME_MAX_LENGTH + 1)
#define IRC_CHANNEL_MAX 32

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

int DelUser(struct IRCAllUsers *allusers, const char *nick);
int AddUser(struct IRCAllUsers *allusers, const char *nick,
            struct Client *thr);

#endif  // ELTEXIRC_SERVER_USERS_H_
