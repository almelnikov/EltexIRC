#include "privmsgsend.h"

int SendMsgToUser(struct IRCAllUsers *allusers, const char *nick,
                  const char *msg) {
  int ret = 0;
  struct IRCUser *ptr;
  struct Client *thr_info;

  pthread_mutex_lock(&allusers->lock);
  ptr = GetUserPtr(allusers, nick);
  if (ptr == NULL) {
    ret = -1;
  } else {
    thr_info = ptr->thr_info;
    pthread_mutex_lock(&thr_info->send_lock);
    write(thr_info->sockfd, msg, strlen(msg));
    pthread_mutex_unlock(&thr_info->send_lock);
  }
  pthread_mutex_unlock(&allusers->lock);
  return ret;
}

int SendMsgToChannel(struct IRCAllChannels *channels,
                     struct IRCAllUsers *allusers, const char *channame,
                     const char *nick, const char *msg) {
  int ret = 0;
  int i;
  struct IRCChannel *chan_ptr;
  struct Client *thr_info;
  char *str;

  pthread_mutex_lock(&channels->lock);
  pthread_mutex_lock(&allusers->lock);
  chan_ptr = GetChannelPtr(channels, channame);
  if (chan_ptr == NULL) {
    ret = -1;
  } else {
    for (i = 0; i < IRC_CHANUSERS_MAX; i++) {
      if (chan_ptr->users[i] != NULL) {
        str = chan_ptr->users[i]->nick;
        if (strncmp(str, nick, IRC_NICK_MAX_LENGTH) != 0) {
          thr_info = chan_ptr->users[i]->thr_info;
          pthread_mutex_lock(&thr_info->send_lock);
          write(thr_info->sockfd, msg, strlen(msg));
          pthread_mutex_unlock(&thr_info->send_lock);
        }
      }
    }
  }
  pthread_mutex_unlock(&allusers->lock);
  pthread_mutex_unlock(&channels->lock);
  return ret;
}
