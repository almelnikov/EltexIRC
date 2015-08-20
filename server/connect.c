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
  char buf[IRC_MSG_SIZE]; //preamble size
  char hostname[HOSTNAME_MAX]; //изменить
  int len = 0, shift = 0, cnt;

  memset(&buf, 0, IRC_MSG_SIZE);
  memset(&hostname, 0, HOSTNAME_MAX);
  //подготовка преамбулы
  buf[shift++] = ':';
  len = strlen(nick);
  for (cnt = 0; cnt < 2; cnt++) {
    strncpy(buf + shift, nick, len);
    shift += len;
    buf[shift++] = '!';
  }
  buf[shift - 1] = '@';

  gethostname(hostname, sizeof hostname);
  len = strlen(hostname);
  if (len + shift >= PREAMBLE_SIZE) {
    return -1;
  } else {
    strncpy(buf + shift, hostname, len);
    shift += len;
  }
  buf[shift++] = ' ';

  //копируем сообщение
  len = strlen(msg);
  strncpy(buf + shift, msg, len);
  shift += len;

 // buf[shift] = '\0';
  memcpy(output, buf, shift);
  output[shift] = '\0';

  return 0;
}
