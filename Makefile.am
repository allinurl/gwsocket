#AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = gwsocket
AUTOMAKE_OPTIONS = subdir-objects

gwsocket_SOURCES = \
  src/base64.c     \
  src/base64.h     \
  src/gslist.c     \
  src/gslist.h     \
  src/gwsocket.c   \
  src/gwsocket.h   \
  src/log.c        \
  src/log.h        \
  src/sha1.c       \
  src/sha1.h       \
  src/websocket.c  \
  src/websocket.h  \
  src/xmalloc.c    \
  src/xmalloc.h

if DEBUG
AM_CFLAGS = -DDEBUG -O0 -g
else
AM_CFLAGS = -O2
endif

AM_LDFLAGS = 
if WITH_RDYNAMIC
AM_LDFLAGS += -rdynamic
endif

if HAVE_OPENSSL
AM_LDFLAGS += -rdynamic
endif

AM_CFLAGS += -Wno-long-long -Wall -W -Wnested-externs -Wformat=2
AM_CFLAGS += -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
AM_CFLAGS += -Wwrite-strings -Wshadow -Wpointer-arith -Wsign-compare
AM_CFLAGS += -Wredundant-decls -Wbad-function-cast -Winline -Wcast-align -Wextra
AM_CFLAGS += -Wdeclaration-after-statement -Wno-missing-field-initializers

dist_man_MANS = gwsocket.1
