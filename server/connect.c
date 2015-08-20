#include "connect.h"

struct ThreadChanNode *CreateThrNode(char *data)
{
  struct ThreadChanNode *node = NULL;
  
  node = (struct ThreadChanNode *)malloc(sizeof(*node));
  if (node != NULL) {
    strcpy(node->chan, data);
    node->next = NULL;
    return node;
  } else {
    return NULL;
  }
}

struct ThreadChanNode *ThrListAddFront(struct ThreadChanList *list,
                                char *chan) {
  struct ThreadChanNode *new = CreateThrNode(chan);
  
  if (new != NULL) {
    new->next = list->head;
    list->size++;
    return new;
  } else {
    return list->head;
  }
}

struct ThreadChanNode *DeleteThrNode(struct ThreadChanList *list,
                                  char *chan)
{
  struct ThreadChanNode *ptr, *prev = NULL;
  
  for (ptr = list->head; ptr != NULL; ptr = ptr->next) {
    if (strcmp(ptr->chan, chan) == 0) {
      if (prev == NULL) {
        list->head = ptr->next;
        free(ptr);
        list->size--;
        return list->head;
      } else {
        prev->next = ptr->next;
        free(ptr);
        list->size--;
        return list->head;
      }
    }
    prev = ptr;
  }
  return NULL;
}

void FreeThreadList(struct ThreadChanList *list)
{
  struct ThreadChanNode *ptr, *prev;
  for (ptr = list->head; ptr != NULL; ) {
    prev = ptr;
    ptr = ptr->next;
    free(prev);
    list->size--;
  }
}

int FormSendMsg(char *output, char *msg, const char *nick)
{
  char buf[IRC_MSG_MAX_LENGTH];
  char hostname[IRC_HOST_MAX_LENGTH];
  int shift = 0, index;
  int nick_len = strlen(nick), host_len = 0, msg_len = 0;
  
  memset(&buf, 0, IRC_MSG_MAX_LENGTH);
  memset(&hostname, 0, IRC_HOST_MAX_LENGTH);
  
  gethostname(hostname, sizeof(hostname));
  host_len = strlen(hostname);
  
  if (host_len + 2 * nick_len + 3 >= IRC_HOST_MAX_LENGTH)
    return -1;
  
  buf[shift++] = ':';
  for (index = 0; index < 2; index++) {
    strncpy(buf + shift, nick, nick_len);
    shift += nick_len;
    buf[shift++] = '!';
  }
  buf[shift - 1] = '@';
  
  strncpy(buf + shift, hostname, host_len);
  shift += host_len;
  
  buf[shift++] = ' ';
  
  msg_len = strlen(msg);
  
  if (shift + msg_len + 2 < IRC_MSG_MAX_LENGTH) {
    strncpy(buf + shift, msg, msg_len);
    shift += msg_len;
  
    strncpy(buf + shift, "\r\n", 2);
    shift += 2;
    
    memcpy(output, buf, shift);
    output[shift] = '\0';
    return 0;
  } else {
    return -1;
  }
}

int IRCMsgRead(int sockfd, char *raw_msg)
{
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


