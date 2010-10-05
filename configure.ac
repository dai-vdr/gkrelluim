#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)

AC_INIT([gkrelluim], 0.3.0, [d+gkrelluim@vdr.jp])
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION, no-define)

AC_CONFIG_SRCDIR([src/gkrelluim.c],[src/uim-helper.c])

# config.h
AC_CONFIG_HEADER(src/config.h)

# Checks for programs.
AC_PROG_CC

# gettext
AC_PROG_INTLTOOL([0.41.1])
GETTEXT_PACKAGE=$PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext domains])
ALL_LINGUAS="ja"
AM_GLIB_GNU_GETTEXT
localedir='${prefix}/share/locale'
AC_SUBST(localedir)

# pkg-config
PKG_CHECK_MODULES(UIM, uim >= 1.5.1)
AC_SUBST(UIM_CFLAGS)
AC_SUBST(UIM_LIBS)
#
PKG_CHECK_MODULES(GKRELLM, gkrellm >= 2.3.0) # not 2.3.1
AC_SUBST(GKRELLM_CFLAGS)
AC_SUBST(GKRELLM_LIBS)

AC_OUTPUT([Makefile src/Makefile po/Makefile.in])
