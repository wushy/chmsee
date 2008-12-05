# Configure paths for chmlib
# Ji YongGang <jungle@soforge-studio.com> 2006-05-15
# Shamelessly stolen from Owen Taylor and Manish Singh

dnl AM_PATH_CHMLIB([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Test for chmlib, and define CHMLIB_CFLAGS and CHMLIB_LIBS
dnl
AC_DEFUN([AM_PATH_CHMLIB],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(chmlib,
        AS_HELP_STRING([--with-chmlib=PFX],[Prefix where chmlib is installed]),
        chmlib_prefix="$withval", 
        chmlib_prefix="")
AC_ARG_WITH(chmlib-libraries,
        AS_HELP_STRING([--with-chmlib-libraries=DIR],[Directory where chmlib library is installed]), 
        chmlib_libraries="$withval", 
        chmlib_libraries="")
AC_ARG_WITH(chmlib-includes,
        AS_HELP_STRING([--with-chmlib-includes=DIR],[Directory where chmlib header files are installed]), 
        chmlib_includes="$withval", 
        chmlib_includes="")
AC_ARG_ENABLE(chmlibtest,
        AS_HELP_STRING([--disable-chmlibtest],[Do not try to compile and run a test CHMLIB program]),
        , 
        enable_chmlibtest=yes)

  if test "x$chmlib_libraries" != "x" ; then
    CHMLIB_LIBS="$chmlib_libraries"
  elif test "x$chmlib_prefix" != "x" ; then
    CHMLIB_LIBS="$chmlib_prefix/lib"
  elif test "x$prefix" != "x" ; then
    CHMLIB_LIBS="$prefix/lib"
  fi

case $target in 
	*bsd*) 
                 CHMLIB_LIBS="-Wl,-R$CHMLIB_LIBS -L$CHMLIB_LIBS" 
                 ;;
         *)
                 CHMLIB_LIBS="-L$CHMLIB_LIBS" 
                 ;;
esac

  CHMLIB_LIBS="$CHMLIB_LIBS -lchm"

  if test "x$chmlib_includes" != "x" ; then
    CHMLIB_CFLAGS="-I$chmlib_includes"
  elif test "x$chmlib_prefix" != "x" ; then
    CHMLIB_CFLAGS="-I$chmlib_prefix/include"
  elif test "x$prefix" != "x"; then
    CHMLIB_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for CHMLIB)
  no_chmlib=""


  if test "x$enable_chmlibtest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"

    CFLAGS="$CFLAGS $CHMLIB_CFLAGS"
    LIBS="$LIBS $CHMLIB_LIBS"

dnl
dnl Now check if the installed CHMLIB is sufficiently new.
dnl
      rm -f conf.chmlibtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chm_lib.h>

int main ()
{
  system("touch conf.chmlibtest");
  return 0;
}

],, no_chmlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_chmlib" = "x" ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])     
  else
     AC_MSG_RESULT(no)
     if test -f conf.chmlibtest ; then
       :
     else
       echo "*** Could not run CHMLIB test program, checking why..."
       CFLAGS="$CFLAGS $CHMLIB_CFLAGS"
       LIBS="$LIBS $CHMLIB_LIBS"
       AC_TRY_LINK([
#include <stdio.h>
#include <chm_lib.h>
],     [ return 0; ],
       [ echo "*** The test program compiled, but did not run. This usually means"
       echo "*** that the run-time linker is not finding CHMLIB or finding the wrong"
       echo "*** version of CHMLIB. If it is not finding CHMLIB, you'll need to set your"
       echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
       echo "*** to the installed location  Also, make sure you have run ldconfig if that"
       echo "*** is required on your system"
       echo "***"
       echo "*** If you have an old version installed, it is best to remove it, although"
       echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
       [ echo "*** The test program failed to compile or link. See the file config.log for the"
       echo "*** exact error that occured. This usually means CHMLIB was incorrectly installed"
       echo "*** or that you have moved CHMLIB since it was installed." ])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
     CHMLIB_CFLAGS=""
     CHMLIB_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(CHMLIB_CFLAGS)
  AC_SUBST(CHMLIB_LIBS)
  rm -f conf.chmlibtest
])
