#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([gwsocket], [0.4], [goaccess@prosoftcorp.com], [], [http://gwsocket.io])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/gwsocket.c])
AC_CONFIG_HEADERS([src/config.h])

# Use empty CFLAGS by default so autoconf does not add
# CFLAGS="-O2 -g"
: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# DEBUG
AC_ARG_ENABLE(debug, [  --enable-debug   Create a debug build. Default is disabled],
  [debug="$enableval"], debug=no)

if test "$debug" = "yes"; then
  AC_DEFINE([_DEBUG], 1, [Debug option])
fi
AM_CONDITIONAL([DEBUG], [test "x$debug" = "xyes"])

# Handle rdynamic only on systems using GNU ld
AC_CANONICAL_HOST
AC_MSG_CHECKING([whether to build with rdynamic for GNU ld])
with_rdyanimc=yes
case "$host_os" in
  *darwin*|*cygwin*|*aix*|*mingw*) with_rdyanimc=no
  ;;
esac
AC_MSG_RESULT([$with_rdyanimc])
AM_CONDITIONAL([WITH_RDYNAMIC], [test "x$with_rdyanimc" = "xyes"])

# Checks for libraries.
AC_ARG_WITH([openssl],AC_HELP_STRING([--with-openssl], [build with OpenSSL support]),
   [openssl="$withval"],[openssl="no"])

# Build with OpenSSL
if test "$openssl" = 'yes'; then
  AC_CHECK_LIB([ssl], [SSL_SESSION_new],,[AC_MSG_ERROR([ssl library missing])])
  AC_CHECK_LIB([crypto], [CRYPTO_free],,[AC_MSG_ERROR([crypto library missing])])
fi
# For Makefile.am
AM_CONDITIONAL(HAVE_OPENSSL, test "$openssl" = "yes")

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([malloc])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mkfifo])
AC_CHECK_FUNCS([poll])
AC_CHECK_FUNCS([realloc])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strpbrk])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

cat << EOF

Your build configuration:

  Prefix         : $prefix
  Package        : $PACKAGE_NAME
  Version        : $VERSION
  Compiler flags : $CFLAGS
  Linker flags   : $LIBS $LDFLAGS
  TLS/SSL        : $openssl
  Bugs           : $PACKAGE_BUGREPORT

EOF
