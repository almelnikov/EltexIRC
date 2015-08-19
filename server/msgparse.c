#include "msgparse.h"

enum IRCCommands GetIRCCommand(const char *str) {
  char *command;
  int length, i;
  enum IRCCommands ret = IRCCMD_UNKNOWN;

  length = strlen(str);
  command = (char*)malloc(length + 1);
  if (command != NULL) {
    command[length] = '\0';
    for (i = 0; i < length; i++) {
      command[i] = toupper(str[i]);
    }

    if (strcmp(command, "PASS") == 0) {
      ret = IRCCMD_PASS;
    } else if (strcmp(command, "USER") == 0) {
      ret = IRCCMD_USER;
    } else if (strcmp(command, "NICK") == 0) {
      ret = IRCCMD_NICK;
    } else if (strcmp(command, "PRIVMSG") == 0) {
      ret = IRCCMD_PRIVMSG;
    } else if (strcmp(command, "JOIN") == 0) {
      ret = IRCCMD_JOIN;
    } else if (strcmp(command, "PART") == 0) {
      ret = IRCCMD_PART;
    } else if (strcmp(command, "QUIT") == 0) {
      ret = IRCCMD_QUIT;
    } else if (strcmp(command, "LIST") == 0) {
      ret = IRCCMD_LIST;
    }

    free(command);
  } else {
    perror("malloc");
  }
  return ret;
}

int FormParsedMsg(const char *str, struct ParsedMsg *msg) {
  const char *ptr = str;
  const char *long_param;
  char *saveptr, *tok_return;
  char buf[1024], tokenized[1024];
  char *tok_ptr = buf;
  char *tmp_ptr;
  int readed_tokens = 0; // Число прочитанных токенов без опционального
  int shift = 0;
  int length;
  int i;

  // Начальные значения
  msg->optional = NULL;
  msg->cnt = 0;
  msg->cmd = IRCCMD_UNKNOWN;
  msg->params = NULL;
  // Убрать пробелы в начале
  while (*ptr == ' ') ptr++;
  // Начало длинного параметра
  long_param = strstr(ptr, " :");
  // Копируем строку до длинного параметра
  if (long_param != NULL) {
    strncpy(buf, ptr, long_param - ptr);
  } else {
    strcpy(buf, ptr);
  }
  // Разбиваем на токены
  while ((tok_return = strtok_r(tok_ptr, " ", &saveptr)) != NULL) {
    if (tok_ptr != NULL && tok_return[0] == ':') { // Есть опциональный параметр
      msg->optional = (char*)malloc(strlen(tok_return) + 1);
      if (msg->optional == NULL) {
        perror("malloc");
        return -1;
      }
      strcpy(msg->optional, tok_return);
    } else {  // Нет опционального параметра
      if (readed_tokens == 0) { // Комманда
        msg->cmd = GetIRCCommand(tok_return);
        readed_tokens++;
      } else { // Параметр
        length = strlen(tok_return) + 1;
        strcpy(tokenized + shift, tok_return);
        shift += length;
        readed_tokens++;
      }
    }
    tok_ptr = NULL;
  }

  if (long_param != NULL) {  // Копируем длинный параметр
    long_param += 2;  // Сдвиг на " :"
    length = strlen(long_param) + 1;
    strcpy(tokenized + shift, long_param);
    shift += length;
    readed_tokens++;
  }

  if (readed_tokens > 0) {  // Есть параметры
    msg->params = malloc((readed_tokens - 1) * sizeof(char*));
    if (msg->params == NULL) {
      perror("malloc");
      return -1;
    }
    tmp_ptr = (char*)malloc(shift);
    if (tmp_ptr == NULL) {
      perror("malloc");
      return -1;
    }
    memcpy(tmp_ptr, tokenized, shift);
    shift = 0;
    for (i = 0; i < (readed_tokens - 1); i++) {
      length = strlen(tmp_ptr + shift) + 1;
      msg->params[i] = tmp_ptr + shift;
      shift += length;
    }
  }
  msg->cnt = readed_tokens - 1;

  return 0;
}

void FreeParsedMsg(struct ParsedMsg *msg) {
  free(msg->optional);
  if (msg->cnt > 0 && msg->params != NULL) {
    free(msg->params[0]);
  }
  free(msg->params);
  msg->cnt = 0;
  msg->params = NULL;
  msg->optional = NULL;
}