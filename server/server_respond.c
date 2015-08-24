#include "server_respond.h"

int SendConnectMsg(struct IRCAllUsers *allusers, const char *host,
                   const char *nick) {
  char buf[IRC_MSG_MAX_LENGTH + 1];

  sprintf(buf, ":%s 001 %s :Welcome to the IRC\r\n", host, nick);
  return SendMsgToUser(allusers, nick, buf);
}

int SendAllChannelsList(int sock, struct IRCAllChannels *channels,
                        struct IRCAllUsers *allusers, const char *host,
                        const char *nick) {

  struct NamesList list, chan_list;
  char buf[IRC_MSG_MAX_LENGTH + 1];
  int len, err;
  int ret = 0;
  int i;
  int users_cnt;

  list = GetChannelsList(channels);
  if (list.names == NULL) {
    return IRCERR_SCL_LIST;
  }
  i = 0;
  for (i = 0; i < list.cnt; i++) {
    err = GetUsersOnChannel(channels, allusers, list.names[i], &chan_list);
    if (err != 0) {
      FreeNamesList(&list);
      return IRCERR_SCL_LIST;
    }
    users_cnt = chan_list.cnt;  // tmp
    sprintf(buf, ":%s 322 %s %s %d : \r\n", host, nick, list.names[i], users_cnt);
    len = strlen(buf);
    if (write(sock, buf, len) != len) {
      ret = IRCERR_SCL_WRITE;
    }
  }
  sprintf(buf, ":%s 323 %s :End of LIST\r\n", host, nick);
  len = strlen(buf);
  if (write(sock, buf, len) != len) {
    ret = IRCERR_SCL_WRITE;
  }
  FreeNamesList(&list);

  return ret;
}

int SendChannelList(int sock, struct IRCAllChannels *channels,
                    struct IRCAllUsers *allusers, const char *channame,
                    const char *host, const char *nick) {

  struct NamesList list;
  char buf[IRC_MSG_MAX_LENGTH + 1];
  int buf_len, len;
  int ret = 0;
  int i;

  if (GetUsersOnChannel(channels, allusers, channame, &list) == 0) {
    i = 0;
    while (i < list.cnt) {
      sprintf(buf, ":%s 353 %s = %s :", host, nick, channame);
      buf_len = strlen(buf);
      while ((buf_len + strlen(list.names[i]) + 1) < (IRC_MSG_MAX_LENGTH - 2)) {
        if (i != 0) {
          strcat(buf, " ");
        }
        strcat(buf, list.names[i]);
        buf_len += strlen(list.names[i]) + 1;
        i++;
        if (i == list.cnt) {
          break;
        }
      }
      strcat(buf, "\r\n");
      len = strlen(buf);
      if (write(sock, buf, len) != len) {
        ret = IRCERR_SCL_WRITE;
      }
    }
    sprintf(buf, ":%s 366 %s %s :End of NAMES list.\r\n", host, nick, channame);
    len = strlen(buf);
    if (write(sock, buf, len) != len) {
        ret = IRCERR_SCL_WRITE;
    }
    FreeNamesList(&list);
  } else {
    ret = IRCERR_SCL_LIST;
  }
  return ret;
}