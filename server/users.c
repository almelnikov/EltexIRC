#include "users.h"

static struct IRCUser *GetUserPtr(struct IRCAllUsers *allusers,
                                  const char *nick) {
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

int AddUser(struct IRCAllUsers *allusers, const char *nick,
            struct Client *thr) {
  struct IRCUser *ptr;
  int ret = 0;
  int duplication_flag = 0;

  pthread_mutex_lock(&allusers->lock);
  if (GetUserPtr(allusers, nick) != NULL) {
    duplication_flag = 1;
  }
  if ((strlen(nick) > IRC_NICK_MAX_LENGTH) || duplication_flag) {
    ret = -1;
  } else {
    ptr = GetUserPtr(allusers, "");
    if (ptr == NULL) {  // Нет свободного элемента
      ret = -1;
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
    ret = -1;
  }
  else {
    strcpy(ptr->nick, "");
  }
  pthread_mutex_unlock(&allusers->lock);
  return ret;
}

int AddUserToChannel() {
  int ret = 0;

  return ret;
}

int RemoveUserFromChannel() {
  int ret = 0;

  return ret;
}
