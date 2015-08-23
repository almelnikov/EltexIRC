#include "server_respond.h"

int SendConnectMsg(struct IRCAllUsers *allusers, const char *host,
                   const char *nick) {
  char buf[IRC_MSG_MAX_LENGTH + 1];

  sprintf(buf, ":%s 001 %s :Welcome to the IRC\r\n", host, nick);
  SendMsgToUser(allusers, nick, buf);
}

int SendChannelList(int sock, struct IRCAllChannels *channels,
                    struct IRCAllUsers *allusers, const char *channame,
                    const char *host, const char *nick) {

  struct NamesList list;
  char buf[IRC_MSG_MAX_LENGTH + 1];
  int flag_exit = 0;
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