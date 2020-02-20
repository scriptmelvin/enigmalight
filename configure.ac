dnl Process this file with autoconf to produce a configure script.

AC_INIT(EnigmaLight,rc3)

AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([src/Include/Config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_MAINTAINER_MODE

AC_HEADER_STDC

AC_ISC_POSIX
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LIBTOOL
AC_SEARCH_LIBS([strerror],[cposix])
PKG_PROG_PKG_CONFIG

dnl all use this
AC_CHECK_LIB(pthread,[main])
AC_CHECK_LIB(rt,[main])
AC_CHECK_LIB(m,[main])
AC_CHECK_LIB(dl,[main])
AC_CHECK_DECL([clock_gettime], AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [ ]), [], [[#include <time.h>]])

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

    AC_CHECK_LIB([usb-1.0],[main],AC_DEFINE([HAVE_LIBUSB], [1], [ ]))
    AC_CHECK_LIB([usb-1.0],[libusb_error_name],AC_DEFINE([HAVE_LIBUSB_ERROR_NAME], [1], [ ]))
    AC_CHECK_HEADERS(libusb.h, [],
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

AC_OUTPUT([
Makefile
src/Makefile
])
