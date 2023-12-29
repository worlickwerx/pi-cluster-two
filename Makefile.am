ACLOCAL_AMFLAGS = -I config

SUBDIRS = .

#if HAVE_SYSTEMD
systemdsystemunit_DATA = etc/bramble.service
#endif

noinst_LIBRARIES = \
    src/libbramble/libbramble.a

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
    src/cmd/version.c \
    src/cmd/usage_bargraph.c \
    src/cmd/shutdown.c

src_cmd_bramble_LDADD = \
    src/libbramble/libbramble.a

src_libbramble_libbramble_a_SOURCES = \
    src/libbramble/bramble.h \
    src/libbramble/utils.c \
    src/libbramble/utils.h \
    src/libbramble/canmsg.h \
    src/libbramble/canlinux.h \
    src/libbramble/canlinux.c \
    src/libbramble/canobj.h \
    src/libbramble/canobj.c \
    src/libbramble/nvram.h \
    src/libbramble/nvram.c \
    src/libbramble/proc.c \
    src/libbramble/proc.h \
    src/libbramble/temp.c \
    src/libbramble/temp.h \
    src/libbramble/version.h

powermanconfdir = $(sysconfdir)/powerman

powermanconf_DATA = \
    etc/bramble.dev

EXTRA_DIST = \
    README.md \
    LICENSE \
    LICENSE.hardware \
    scripts/install-deps-deb.sh \
    scripts/debbuild.sh \
    $(powermanconf_DATA)

export DEB_BUILD_OPTIONS ?= nocheck terse
deb: debian scripts/debbuild.sh
	+@$(top_srcdir)/scripts/debbuild.sh $(abs_top_srcdir)
