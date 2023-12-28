AC_INIT([bramble],
        m4_esyscmd([git describe --always | awk '/.*/ {sub(/^v/, ""); printf "%s",$1; exit}']))
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects tar-pax foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config/config.h])
AM_MAINTAINER_MODE([enable])

AC_DEFINE([_GNU_SOURCE], 1,
  [Define _GNU_SOURCE so that we get all necessary prototypes])

##
# Generate project versions from PACKAGE_VERSION (set from git describe above)
##
AX_SPLIT_VERSION
AX_POINT_VERSION=$(echo $AX_POINT_VERSION | $SED 's/-.*$//')
AC_SUBST([AX_MAJOR_VERSION])
AC_SUBST([AX_MINOR_VERSION])
AC_SUBST([AX_POINT_VERSION])

AC_PROG_CC
AM_PROG_CC_C_O

X_AC_EXPAND_INSTALL_DIRS
RRA_WITH_SYSTEMD_UNITDIR

LT_INIT

AC_SEARCH_LIBS([ev_run], [ev], [], [
  AC_MSG_ERROR([Please install libev development package])
])

AC_CONFIG_FILES( \
  Makefile \
  etc/bramble.service \
  src/libbramble/version.h
)

AC_OUTPUT
