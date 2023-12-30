AC_INIT([bramble-firmware],
        m4_esyscmd([git describe --always | awk '/.*/ {sub(/^v/, ""); printf "%s",$1; exit}']))
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects tar-pax foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config/config.h])
AM_MAINTAINER_MODE([enable])

# The target needs this, not the AC_PROGS_CC default CFLAGS="-g -O2"
test -n "$CFLAGS" || CFLAGS="-Os -g"

# Tweak LDFLAGS while AC_PROG_CC checks that the C compiler can make
# executables, otherwise this fails when cross-compiling.
saved_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS --static -nostartfiles"
AC_PROG_CC
LDFLAGS="$saved_LDFLAGS"

AC_PROG_RANLIB
AC_CHECK_TOOL([SIZE], [size])

AC_CONFIG_FILES( \
  Makefile \
)

AC_OUTPUT
