ACLOCAL_AMFLAGS = -I config

noinst_LTLIBRARIES = \
    src/libbramble/libbramble.la

bin_PROGRAMS = \
    src/cmd/bramble

src_cmd_bramble_SOURCES = \
    src/cmd/bramble.c

src_cmd_bramble_LDADD = \
    src/libbramble/libbramble.la

src_libbramble_libbramble_la_SOURCES = \
    src/libbramble/bramble.h
