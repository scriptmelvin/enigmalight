# -*- mode: m4; -*-
# Process this file with autoconf to produce a configure script

AC_INIT([EnigmaLight],[rc4])
AC_CONFIG_HEADERS([src/Include/Config.h])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CXX
AC_LANG([C++])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
AM_PROG_AR
PKG_PROG_PKG_CONFIG

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
LT_INIT

dnl all use this
AC_CHECK_LIB([pthread], [main],,)
AC_CHECK_LIB([rt], [main],,)
AC_CHECK_LIB([m], [main],,)
AC_CHECK_LIB([dl], [main],,)
AC_CHECK_DECL([clock_gettime], [AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [ ])], [ ], [#include <time.h>])

# Special check for pthread support
AX_PTHREAD([
 AC_DEFINE(HAVE_PTHREAD, 1,
  [Define if you have POSIX threads libraries and header files.])
],[
 AC_MSG_ERROR([POSIX thread support required])
])

CC="$PTHREAD_CC"

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)

# check for pthread_cancel()
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
AC_CHECK_FUNCS(pthread_cancel)
CFLAGS="$saved_CFLAGS"

# --disable-libusb
AC_ARG_ENABLE(libusb,
    AS_HELP_STRING([--disable-libusb],[do not use libusb]),
    [ use_libusb="${enableval}" ], [ use_libusb=yes ] )

# check if libusb is used
LIBUSB_NEEDED_VERSION="1.0.9"
if test "x$use_libusb" != xno ; then
    PKG_CHECK_EXISTS([libusb-1.0], [
    PKG_CHECK_MODULES(LIBUSB, libusb-1.0 >= $LIBUSB_NEEDED_VERSION, [],
        [
        AC_MSG_WARN([install libusb $LIBUSB_NEEDED_VERSION or later])
        PKG_CHECK_MODULES(LIBUSB, libusb-1.0)
        ])
    ])

    saved_CPPFLAGS="$CPPFLAGS"
    saved_LIBS="$LIBS"

    CPPFLAGS="$CPPFLAGS $LIBUSB_CFLAGS"
    LIBS="$LDFLAGS $LIBUSB_LIBS"
    CPPFLAGS="$CPPFLAGS $LIBUSB_CFLAGS"
	LIBS="$LDFLAGS $LIBUSB_LIBS"

    AC_CHECK_LIB([usb-1.0],[main],AC_DEFINE([HAVE_LIBUSB], [1], [ ]))
    AC_CHECK_LIB([usb-1.0],[libusb_error_name],AC_DEFINE([HAVE_LIBUSB_ERROR_NAME], [1], [ ]))
    AC_CHECK_HEADER(libusb.h, [],
        [ AC_MSG_ERROR([libusb.h not found, install libusb or use LIBUSB_CFLAGS=... ./configure ]) ])

    AC_MSG_CHECKING([for libusb_init])
    AC_TRY_LINK_FUNC(libusb_init, [ AC_MSG_RESULT([yes]) ],
        [ AC_MSG_ERROR([libusb not found, use ./configure LIBUSB_LIBS=...]) ])

    CPPFLAGS="$saved_CPPFLAGS"
    LIBS="$saved_LIBS"

    use_libusb=yes
fi
AC_SUBST(LIBUSB_CFLAGS)
AC_SUBST(LIBUSB_LIBS)
AM_CONDITIONAL(WITH_LIBUSB, test "${use_libusb}" != "no")

# C Compiler features
AC_C_INLINE
if test "$GCC" = "yes"; then
    CFLAGS="-Wall -fno-common $CFLAGS"
fi

# check if the compiler support -fvisibility=hidden (GCC >= 4)
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fvisibility=hidden"
AC_MSG_CHECKING([for -fvisibility=hidden])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([char foo;])],
    [	AC_MSG_RESULT([yes])
	SYMBOL_VISIBILITY="-fvisibility=hidden" ],
    AC_MSG_RESULT([no]))
CFLAGS="$saved_CFLAGS"
AC_SUBST(SYMBOL_VISIBILITY)

AC_SEARCH_LIBS([dlopen], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlopen() function])
])

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT
