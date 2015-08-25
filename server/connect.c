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
    
  snprintf(buf, IRC_MSG_MAX_LENGTH, ":%s!%s@%s ", nick, nick, hostname);
  
  shift = strlen(buf);
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

int FormPongMsg(struct IRCAllUsers *users, char *raw_msg, char *nick)
{
  char send_msg[IRC_MSG_MAX_LENGTH];
  
  memset(&send_msg, 0, IRC_MSG_MAX_LENGTH);
  
  raw_msg[1] = 'O';
  if (FormSendMsg(send_msg, raw_msg, nick) == 0)
    SendMsgToUser(users, nick, send_msg);
    
  return 0;
}

int ErrorHandler(int sockfd, char *raw_msg, int numeric)
{
  char hostname[IRC_HOST_MAX_LENGTH];
  char buf[IRC_MSG_MAX_LENGTH];
  size_t len = 0, bytes = 0;
  
  memset(&hostname, 0, IRC_HOST_MAX_LENGTH);
  memset(&buf, 0, IRC_MSG_MAX_LENGTH);
  
  gethostname(hostname, IRC_HOST_MAX_LENGTH - 1);
  
  snprintf(buf, IRC_MSG_MAX_LENGTH, ":%s %d : %s\r\n", hostname, numeric, raw_msg);
  
  printf("Error Msg: %s\n", buf);
  len = strlen(buf);
  
  if ((bytes = write(sockfd, buf, len)) > 0)
    return bytes;
  else 
    return -1;
}
