#include "ircstr.h"

static int IsLetter(char c) {
  if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
    return 1;
  } else {
    return 0;
  }
}

static int IsSpecialChar(char c) {
  if (((c >= 0x5B) && (c <= 0x60)) || ((c >= 0x7B) && (c <= 0x7D))) {
    return 1;
  } else {
    return 0;
  }
}

static int IsFirstNickChar(char c) {
  if (IsLetter(c) || IsSpecialChar(c)) {
    return 1;
  } else {
    return 0;
  }
}

static int IsSecondNickChar(char c) {
  if (IsLetter(c) || IsSpecialChar(c) || (c == '-') || ((c >= '0') && (c <= '9'))) {
    return 1;
  } else {
    return 0;
  }
}

static int IsChannelChar(char c) {
  const char kInvalidChars[] = {'\0', 0x07, '\r', '\n', ' ', ',', ':'};
  const int kInvalidCnt = 7;
  int i;

  for (i = 0; i < kInvalidCnt; i++) {
    if (c == kInvalidChars[i]) return 0;
  }
  return 1;
}

char IRCUppercase(char c) {
  char upper;

  if (c == '{') {
    upper = '[';
  } else if (c == '}') {
    upper = ']';
  } else if (c == '|') {
    upper = '\\';
  } else if (c == '^') {
    upper = '~';
  } else {
    upper = toupper(c);
  }
  return upper;
}

int IRCStrcmp(const char *str1, const char *str2) {
  unsigned char c1, c2;

  for (; IRCUppercase(*str1) == IRCUppercase(*str2); str1++, str2++) {
    if (*str1 == '\0') return 0;
  }
  c1 = (unsigned char)IRCUppercase(*str1);
  c2 = (unsigned char)IRCUppercase(*str2);
  return c1 < c2 ? -1 : 1;
}

int IsValidNick(const char *nick) {
  int ret = 1;
  int i;

  if (strlen(nick) > IRC_NICK_MAX_LENGTH) {
    ret = 0;
  } else if (!IsFirstNickChar(nick[0])) {
    ret = 0;
  } else {
    for (i = 1; nick[i] != '\0'; i++) {
      if (!IsSecondNickChar(nick[i])) {
        ret = 0;
        break;
      }
    }
  } 
  return ret;
}

int IsValidChannel(const char *chan) {
  int ret = 1;
  int i;

  if (strlen(chan) > IRC_CHANNAME_MAX_LENGTH || strlen(chan) < 2) {
    ret =  0;
  } else if (chan[0] != '#') {
    ret = 0;
  } else {
    for (i = 1; chan[i] != '\0'; i++) {
      if (!IsChannelChar(chan[i])) {
        ret = 0;
        break;
      }
    }
  }
  return ret;
}
