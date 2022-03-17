ACLOCAL_AMFLAGS = -I config

#if HAVE_SYSTEMD
systemdsystemunit_DATA = etc/bramble.service
#endif

noinst_LTLIBRARIES = \
    src/libbramble/libbramble.la

bin_PROGRAMS = \
    src/cmd/bramble

src_cmd_bramble_SOURCES = \
    src/cmd/bramble.c \
    src/cmd/cansnoop.c \
    src/cmd/canping.c \
    src/cmd/slot.c \
    src/cmd/led.c \
    src/cmd/power.c \
    src/cmd/console.c \
    src/cmd/firmware-version.c \
    src/cmd/usage_bargraph.c

src_cmd_bramble_LDADD = \
    src/libbramble/libbramble.la

src_libbramble_libbramble_la_SOURCES = \
    src/libbramble/bramble.h \
    src/libbramble/utils.c \
    src/libbramble/utils.h \
    src/libbramble/canmsg.h \
    src/libbramble/canlinux.h \
    src/libbramble/canlinux.c \
    src/libbramble/canobj.h \
    src/libbramble/canobj.c \
    src/libbramble/i2clinux.h \
    src/libbramble/i2clinux.c \
    src/libbramble/i2cproto.h \
    src/libbramble/proc.c \
    src/libbramble/proc.h

powermanconfdir = $(sysconfdir)/powerman

powermanconf_DATA = \
    etc/bramble.dev

EXTRA_DIST = $(powermanconf_DATA)
