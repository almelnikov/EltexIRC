
AC_PREREQ([2.69])
AC_INIT([IRC server & client], [0.1], [alenadedigurova@yandex.ru])
AM_INIT_AUTOMAKE([])
AM_MAINTAINER_MODE([disable])
AC_CONFIG_SRCDIR([server/main.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([pthread.h curses.h sys/socket.h netinet/in.h arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT([Makefile] [server/Makefile] [lib/Makefile] [client/Makefile])
