#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([chmsee],[1.0.4-SNAPSHOT],[http://code.google.com/p/chmsee/issues/entry])
AM_MAINTAINER_MODE
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR([src/chmsee.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_RANLIB
AC_ISC_POSIX
AC_HEADER_STDC
AC_PROG_INSTALL

PKG_CHECK_MODULES(
        CHMSEE_UTILS,
        [glib-2.0]
        )

PKG_CHECK_MODULES(
        CHMSEE_MODELS,
        [glib-2.0,
         libxml-2.0]
         )
                                

PKG_CHECK_MODULES(CHMSEE, [
        gthread-2.0 >= 2.8.0
        gtk+-2.0 >= 2.8.0
        libglade-2.0 >= 2.4.0
])

# Check for Gecko

AC_ARG_WITH(gecko,
    AS_HELP_STRING([--with-gecko=ARG],[specify which Gecko provider to use ("mozilla", "firefox", "seamonkey", "xulrunner" or "libxul"), default is "libxul"]),
    ,
    [with_gecko=libxul])

# Package maintainer should change this to proper value           
if test "x$with_gecko" = "xlibxul"; then
    gecko_provider=libxul-embedding-unstable
else
    gecko_provider=$with_gecko-gtkmozembed
fi

PKG_CHECK_MODULES(GECKO, $gecko_provider, enable_gecko=yes, enable_gecko=no)

AM_CONDITIONAL(WITH_GECKO, test "x$enable_gecko" = "xyes")

if test "x$enable_gecko" != "xno" ; then

    GECKO_INCLUDE_ROOT=`$PKG_CONFIG --cflags-only-I $gecko_provider | awk '{print $1}' | sed "s/^-I//"`
    GECKO_LIB_ROOT=`$PKG_CONFIG --libs-only-L $gecko_provider | awk '{print $1}' |  cut -c 3-`
    GECKO_INCLUDE_ROOT=`dirname $GECKO_INCLUDE_ROOT`
    GECKO_FLAVOUR=$with_gecko

    GECKO_CFLAGS="$GECKO_CFLAGS \
-I$GECKO_INCLUDE_ROOT \
-I$GECKO_INCLUDE_ROOT/commandhandler \
-I$GECKO_INCLUDE_ROOT/content \
-I$GECKO_INCLUDE_ROOT/dom \
-I$GECKO_INCLUDE_ROOT/find \
-I$GECKO_INCLUDE_ROOT/gfx \
-I$GECKO_INCLUDE_ROOT/gtkembedmoz \
-I$GECKO_INCLUDE_ROOT/locale \
-I$GECKO_INCLUDE_ROOT/pref \
-I$GECKO_INCLUDE_ROOT/string \
-I$GECKO_INCLUDE_ROOT/webbrwsr \
-I$GECKO_INCLUDE_ROOT/xpcom \
-I$GECKO_INCLUDE_ROOT/xpconnect"

    AC_SUBST(GECKO_CFLAGS)
    AC_SUBST(GECKO_LIBS)   
    AC_SUBST(GECKO_LIB_ROOT)
    AC_SUBST(GECKO_FLAVOUR)
    AC_DEFINE_UNQUOTED([GECKO_LIB_ROOT], ["${GECKO_LIB_ROOT}"], [Gecko component path])
else

    AC_MSG_ERROR([
*** You must have either the Mozilla, Firefox, Seamonkey or XulRunner
*** development libraries installed in order to build ChmSee!
])
fi

# Check for chmlib library
AM_PATH_CHMLIB(, AC_MSG_ERROR([Can't find/use chmlib. Please install CHMLIB first.])
)

# Check md5 implement
case $target in 
	*linux*) 
		AM_PATH_LIBGCRYPT(,,AC_MSG_ERROR([Can't find/use libgcrypt. Please install libgcrypt first.]))
		MD5_LIBS="$LIBGCRYPT_LIBS" 
		;;
	*freebsd*) 
		MD5_LIBS="-lmd" 
		;;
         *)
		MD5_LIBS=
		;;
esac
AC_SUBST(MD5_LIBS)        

# INTLOOLS
IT_PROG_INTLTOOL(0.37)

GETTEXT_PACKAGE=chmsee
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
ALL_LINGUAS="bn he it sv sw zh_CN"
AM_GLIB_GNU_GETTEXT

AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

# Debug option
AC_ARG_ENABLE(debug,
        AS_HELP_STRING([--enable-debug],[turn on debug]),
        [CFLAGS=`echo "$CFLAGS" | sed -e 's/-O.//g'`
         CFLAGS="$CFLAGS -Wall -D_DEBUG=1"
         enable_debug=yes],
        enable_debug=no)

AC_SUBST([AM_CXXFLAGS])

AC_CONFIG_FILES([
        Makefile
	data/Makefile
        src/Makefile
        src/utils/Makefile
        src/models/Makefile
        cs2w/Makefile
        cs2w/template/Makefile
        cs2w/template/cs2w_icons/Makefile
        cs2w/template/stylesheet/Makefile
        doc/Makefile
        po/Makefile.in])

AC_OUTPUT

AC_MSG_RESULT([
====================================================================
  ChmSee build options:                                            
====================================================================

        Target                    $target  
        Enable debug              $enable_debug
        Gecko provider            $with_gecko
])
