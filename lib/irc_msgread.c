#include "irc_msgread.h"

int IRCMsgRead(int sockfd, char *raw_msg) {
  char buf[IRC_MSG_MAX_LENGTH];
  int bytes, total = 0;
  for (;;) {
    if (total >= IRC_MSG_MAX_LENGTH)
      return -1;
    if ((bytes = read(sockfd, &buf[total], 1)) <= 0)
      return -1;

    if (buf[total] == '\r') {
      if (++total < IRC_MSG_MAX_LENGTH) {
        if (read(sockfd, &buf[total], 1) <= 0)
          return -1;
        if (buf[total] == '\n') {
          buf[total - 1] = '\0';
          strncpy(raw_msg, buf, total);
          return total - 1;
        } else
          return -1;
      } else {
        return -1;
      }
    } else {
      total += bytes;
    }
  }
}