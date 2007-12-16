#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([gkrelluim], 0.1.2, [d+gkrelluim@vdr.jp])
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION, no-define)

AC_CONFIG_SRCDIR([src/gkrelluim.c],[src/uim-helper.c])

# config.h
AC_CONFIG_HEADER(src/config.h)

# Checks for programs.
AC_PROG_CC

# gettext
ALL_LINGUAS="ja"
AM_GNU_GETTEXT
LDFLAGS="$LDFLAGS $INTLLIBS"
localedir='${prefix}/share/locale'
AC_SUBST(localedir)

# pkg-config
PKG_CHECK_MODULES(UIM, uim >= 1.1.0)
AC_SUBST(UIM_CFLAGS)
AC_SUBST(UIM_LIBS)
#
PKG_CHECK_MODULES(GKRELLM, gkrellm >= 2.2.9)
AC_SUBST(GKRELLM_CFLAGS)
AC_SUBST(GKRELLM_LIBS)

AC_OUTPUT([Makefile src/Makefile intl/Makefile po/Makefile.in])
