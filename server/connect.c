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

int FormChanList(struct IRCAllChannels *channels, struct IRCAllUsers *users,
                const char *nick)
{
  struct NamesList list = {
    .cnt = 0,
    .names = NULL
  };
  const int kFormatSize = IRC_HOST_MAX_LENGTH + IRC_NICK_BUF_SIZE + IRC_CHANNAME_MAX_LENGTH + 3;
  const int kPreambleSize = IRC_CHANNAME_MAX_LENGTH + IRC_NICK_MAX_LENGTH + 3;
  const char *default_msg = ":default\n";
  char send_msg[IRC_MSG_MAX_LENGTH];
  char raw_msg[IRC_MSG_MAX_LENGTH];
  char host[IRC_HOST_MAX_LENGTH];
  char preamble[kPreambleSize];
  char format_str[kFormatSize];
  int index, len = 0, shift = 0, nick_len = strlen(nick);
  int preamble_len = 0;
  int default_len = strlen(default_msg);
  
  memset(&preamble, 0, kPreambleSize);
  memset(&raw_msg, 0, IRC_MSG_MAX_LENGTH);
  memset(&send_msg, 0, IRC_MSG_MAX_LENGTH);
  memset(&format_str, 0, kFormatSize);
  memset(&host, 0, IRC_HOST_MAX_LENGTH);
  
  gethostname(host, IRC_HOST_MAX_LENGTH - 1);
  if (len + nick_len + 3 >= IRC_HOST_MAX_LENGTH) 
    return -1;
    
  sprintf(preamble, ":%s %s ", host, nick);
  preamble_len = strlen(preamble);
  
  list = GetChannelsList(channels);
  if (list.cnt == 0) {
    FreeNamesList(&list);
    return 0;
  }
  shift = 0;
  strncpy(raw_msg, "LIST ", 5);
  shift += 5;
  
  for (index = 0; index < list.cnt; index++) {
    len = strlen(list.names[index]);
    if (shift + preamble_len + default_len + len + 2 < IRC_MSG_MAX_LENGTH) {
      snprintf(format_str, kFormatSize, "%s%s %s", 
                preamble, list.names[index], default_msg);
      strncpy(raw_msg + shift, format_str, strlen(format_str));
      shift += strlen(format_str);
    } else {
      //добавить обработку переполнения
      FreeNamesList(&list);
      return -1;
    }
  }
  printf("%d\n", shift);
  raw_msg[shift] = '\0';
  if (FormSendMsg(send_msg, raw_msg, nick) == 0) 
    SendMsgToUser(users, nick, send_msg);
  printf("send list: %s\n", send_msg);
  FreeNamesList(&list);
  return 0;
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
