#ifndef ELTEXIRC_SERVER_IRCSTR_H_
#define ELTEXIRC_SERVER_IRCSTR_H_

#include <ctype.h>
#include <string.h>
#include "irc_limits.h"

char IRCUppercase(char c);
int IRCStrcmp(const char *str1, const char *str2);
int IsValidNick(const char *nick);
int IsValidChannel(const char *chan);

#endif  // ELTEXIRC_SERVER_IRCSTR_H_
