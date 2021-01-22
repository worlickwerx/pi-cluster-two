ACLOCAL_AMFLAGS = -I config

noinst_LTLIBRARIES = \
    src/libbramble/libbramble.la

bin_PROGRAMS = \
    src/cmd/bramble

src_cmd_bramble_SOURCES = \
    src/cmd/bramble.c \
    src/cmd/cansnoop.c \
    src/cmd/slot.c \
    src/cmd/power.c \
    src/cmd/firmware-version.c

src_cmd_bramble_LDADD = \
    src/libbramble/libbramble.la

src_libbramble_libbramble_la_SOURCES = \
    src/libbramble/bramble.h \
    src/libbramble/utils.c \
    src/libbramble/utils.h \
    src/libbramble/canmsg.h \
    src/libbramble/canmsg_v1.h \
    src/libbramble/canmsg_v1.c \
    src/libbramble/canlinux.h \
    src/libbramble/canlinux.c \
    src/libbramble/canobj.h \
    src/libbramble/canobj.c \
    src/libbramble/i2clinux.h \
    src/libbramble/i2clinux.c
