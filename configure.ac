
AC_PREREQ([2.69])
AC_INIT([IRC server & client], [0.1], [alenadedigurova@yandex.ru])
AM_INIT_AUTOMAKE([])
AM_MAINTAINER_MODE([disable])
AC_CONFIG_SRCDIR([server/main.c])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([ncurses], [newiwn])
AC_CHECK_LIB([pthread], [pthread_mutex_lock])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostname memset socket strchr strstr])

AC_OUTPUT([Makefile] [server/Makefile] [lib/Makefile] [client/Makefile])
