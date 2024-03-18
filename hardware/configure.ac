AC_INIT([bramble-hardware],
        m4_esyscmd([git describe --always | awk '/.*/ {sub(/^v/, ""); printf "%s",$1; exit}']))
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects tar-pax foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config/config.h])
AM_MAINTAINER_MODE([enable])

AC_ARG_WITH([kicad],
  AS_HELP_STRING([--without-kicad],
    [do not run kicad design/electrical rules checks on make check]))

dnl build with kicad unless --without-kicad was specified
AS_IF([test "x$with_kicad" != "xno"],
  [AC_PATH_PROG([KICAD_CLI], [kicad-cli])
  AS_IF([test "$KICAD_CLI"], [have_kicad=yes], [have_kicad=no])],
  [have_kicad=no])

dnl make sure it's at least 8.0.1 for the new cli subcommands we use
AS_IF([test "x$have_kicad" = "xyes"],
  [KICAD_CLI_VERSION=`kicad-cli version`
  AX_COMPARE_VERSION([$KICAD_CLI_VERSION], [ge], [8.0.1],
    [AC_MSG_NOTICE([kicad-cli $KICAD_CLI_VERSION is acceptable])],
    [AC_MSG_NOTICE([kicad-cli $KICAD_CLI_VERSION is too old])
      have_kicad=no])
])

dnl fail if --with-kicad was specified but it was not found
AS_IF([test "x$have_kicad" != "xyes"],
  [AS_IF([test "x$with_kicad" = "xyes"],
    [AC_MSG_ERROR([kicad requested but not found])
  ])
])

AM_CONDITIONAL([HAVE_KICAD], [test "x$have_kicad" = "xyes"])

AC_CONFIG_FILES( \
  Makefile
)

AC_OUTPUT
