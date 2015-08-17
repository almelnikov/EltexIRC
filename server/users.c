#include "users.h"

struct IRCUser *GetUserPtr(struct IRCAllUsers *allusers, const char *nick) {
  int i;
  struct IRCUser *ret = NULL;
  char *str;

  for (i = 0; i < IRC_USERS_MAX; i++) {
    str = allusers->users[i].nick;
    if (strncmp(str, nick, IRC_NICK_MAX_LENGTH) == 0) {
      ret = &allusers->users[i];
      break;
    }
  }
  return ret;
}

int RenameUser(struct IRCAllUsers *allusers, const char *oldnick,
               const char *newnick) {
  struct IRCUser *ptr;
  int ret = 0;

  pthread_mutex_lock(&allusers->lock);
  ptr = GetUserPtr(allusers, oldnick);
  if (ptr == NULL) {
    ret = IRC_USERERR_NOTFOUND;
  } else {
    strcpy(ptr->nick, newnick);
  }
  pthread_mutex_unlock(&allusers->lock);
  return ret;
}

int AddUser(struct IRCAllUsers *allusers, const char *nick,
            struct Client *thr) {
  struct IRCUser *ptr;
  int ret = 0;
  int duplication_flag = 0;

  pthread_mutex_lock(&allusers->lock);
  if (GetUserPtr(allusers, nick) != NULL) {
    duplication_flag = 1;
  }
  if (duplication_flag) {
    ret = IRC_USERERR_EXIST;
  } else {
    ptr = GetUserPtr(allusers, "");
    if (ptr == NULL) {  // Нет свободного элемента
      ret = IRC_USERERR_CANTADD;
    } else {
      strcpy(ptr->nick, nick);
      ptr->thr_info = thr;
    }
  }
  pthread_mutex_unlock(&allusers->lock);
  return ret;
}

int DelUser(struct IRCAllUsers *allusers, const char *nick) {
  struct IRCUser *ptr;
  int ret = 0;

  pthread_mutex_lock(&allusers->lock);
  ptr = GetUserPtr(allusers, nick);
  if (ptr == NULL) {
    ret = IRC_USERERR_NOTFOUND;
  }
  else {
    strcpy(ptr->nick, "");
  }
  pthread_mutex_unlock(&allusers->lock);
  return ret;
}

struct IRCChannel *GetChannelPtr(struct IRCAllChannels *channels,
                                 const char *channame) {
  int i;
  struct IRCChannel *ret = NULL;
  char *str;

  for (i = 0; i < IRC_CHANNEL_MAX; i++) {
    str = channels->channels[i].name;
    if (strncmp(str, channame, IRC_CHANNAME_MAX_LENGTH) == 0) {
      ret = &channels->channels[i];
      break;
    }
  }
  return ret;
}

struct IRCUser **FindUserOnChan(struct IRCChannel *chan, const char *nick) {
  int i;
  struct IRCUser **ret = NULL;
  char *str;

  for (i = 0; i < IRC_CHANUSERS_MAX; i++) {
    if (chan->users[i] != NULL) {
      str = chan->users[i]->nick;
      if (strncmp(str, nick, IRC_NICK_MAX_LENGTH) == 0) {
        ret = &chan->users[i];
        break;
      }
    }
  }

  return ret;
}

static struct IRCUser **FindEmptyChanSpace(struct IRCChannel *chan) {
  int i;
  struct IRCUser **ret = NULL;

  for (i = 0; i < IRC_CHANUSERS_MAX; i++) {
    if (chan->users[i] == NULL) {
      ret = &chan->users[i];
      break;
    }
  }

  return ret;
}

int AddUserToChannel(struct IRCAllChannels *channels,
                     struct IRCAllUsers *allusers,
                     const char *channame, const char *nick) {
  int ret = 0;
  struct IRCChannel *chan_ptr;
  struct IRCUser *user_ptr;
  struct IRCUser **duser_ptr;

  pthread_mutex_lock(&channels->lock);
  pthread_mutex_lock(&allusers->lock);
  user_ptr = GetUserPtr(allusers, nick);
  pthread_mutex_unlock(&allusers->lock);

  chan_ptr = GetChannelPtr(channels, channame);
  if (chan_ptr == NULL) {
    // Юзер первым заходит на канал и создает его
    chan_ptr = GetChannelPtr(channels, "");
    if (chan_ptr == NULL) {
      ret = IRC_USERERR_CANTADD;
    }
    else {
      strncpy(chan_ptr->name, channame, IRC_CHANNAME_MAX_LENGTH);
      duser_ptr = FindEmptyChanSpace(chan_ptr);  // Считается что выполнена всегда
      *duser_ptr = user_ptr;
    }
  } else {
    duser_ptr = FindEmptyChanSpace(chan_ptr);
    if (duser_ptr == NULL) {
      ret = IRC_USERERR_CANTADD;
    }
    else {
      *duser_ptr = user_ptr;
    }
  }
  pthread_mutex_unlock(&channels->lock);
  return ret;
}

int RemoveUserFromChannel(struct IRCAllChannels *channels, const char *channame,
                          const char *nick) {
  int ret = 0;
  struct IRCChannel *chan_ptr;
  struct IRCUser **duser_ptr;

  pthread_mutex_lock(&channels->lock);
  chan_ptr = GetChannelPtr(channels, channame);
  if (chan_ptr == NULL) {
    ret = IRC_USERERR_NOTFOUND;
  } else {
    duser_ptr = FindUserOnChan(chan_ptr, nick);
    if (duser_ptr == NULL) {
      ret = IRC_USERERR_NOTFOUND;
    }
    else {
      *duser_ptr = NULL;
    }
  }
  pthread_mutex_unlock(&channels->lock);

  return ret;
}

void UsersInit(struct IRCAllUsers *users) {
  memset(users, 0, sizeof(*users));
  pthread_mutex_init(&users->lock, NULL);
}

void ChannelsInit(struct IRCAllChannels *channels) {
  memset(channels, 0, sizeof(*channels));
  pthread_mutex_init(&channels->lock, NULL);
}
