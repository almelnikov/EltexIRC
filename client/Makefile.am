bin_PROGRAMS=irc_client
irc_client_SOURCES=client.c

AM_CPPFLAGS = -I$(top_srcdir)/lib
LDADD=$(top_srcdir)/lib/libeltirc.a -lpthread -lncurses
