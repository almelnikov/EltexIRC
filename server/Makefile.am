bin_PROGRAMS=irc
irc_SOURCES=main.c connect.c connect.h thread_info.h \
            users.c users.h privmsgsend.c privmsgsend.h server_respond.c \
            server_respond.h

AM_CPPFLAGS = -I$(top_srcdir)/lib
LDADD=$(top_srcdir)/lib/libeltirc.a -lpthread
