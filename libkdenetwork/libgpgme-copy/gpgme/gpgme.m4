dnl Autoconf macros for libgpgme
dnl $Id$

AC_DEFUN(_AM_PATH_GPGME_CONFIG,
[ AC_ARG_WITH(gpgme-prefix,
            AC_HELP_STRING([--with-gpgme-prefix=PFX],
                           [prefix where GPGME is installed (optional)]),
     gpgme_config_prefix="$withval", gpgme_config_prefix="")
  if test "x$gpgme_config_prefix" != x ; then
      GPGME_CONFIG="$gpgme_config_prefix/bin/gpgme-config"
  fi
  AC_PATH_PROG(GPGME_CONFIG, gpgme-config, no)

  gpgme_version=`$GPGME_CONFIG --version`
  gpgme_version_major=`echo $gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
  gpgme_version_minor=`echo $gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
  gpgme_version_micro=`echo $gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`
])

dnl AM_PATH_GPGME([MINIMUM-VERSION,
dnl               [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libgpgme and define GPGME_CFLAGS and GPGME_LIBS.
dnl
AC_DEFUN([AM_PATH_GPGME],
[ AC_REQUIRE([_AM_PATH_GPGME_CONFIG])dnl
  min_gpgme_version=ifelse([$1], ,0.4.2,$1)
  AC_MSG_CHECKING(for GPGME - version >= $min_gpgme_version)
  ok=no
  if test "$GPGME_CONFIG" != "no" ; then
    req_major=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\1/'`
    req_minor=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\2/'`
    req_micro=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\3/'`
    if test "$gpgme_version_major" -gt "$req_major"; then
        ok=yes
    else 
        if test "$gpgme_version_major" -eq "$req_major"; then
            if test "$gpgme_version_minor" -gt "$req_minor"; then
               ok=yes
            else
               if test "$gpgme_version_minor" -eq "$req_minor"; then
                   if test "$gpgme_version_micro" -ge "$req_micro"; then
                     ok=yes
                   fi
               fi
            fi
        fi
    fi
  fi
  if test $ok = yes; then
    GPGME_CFLAGS=`$GPGME_CONFIG --cflags`
    GPGME_LIBS=`$GPGME_CONFIG --libs`
    AC_MSG_RESULT(yes)
    ifelse([$2], , :, [$2])
  else
    GPGME_CFLAGS=""
    GPGME_LIBS=""
    AC_MSG_RESULT(no)
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GPGME_CFLAGS)
  AC_SUBST(GPGME_LIBS)
])

dnl AM_PATH_GPGME_PTH([MINIMUM-VERSION,
dnl                   [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libgpgme and define GPGME_PTH_CFLAGS and GPGME_PTH_LIBS.
dnl
AC_DEFUN([AM_PATH_GPGME_PTH],
[ AC_REQUIRE([_AM_PATH_GPGME_CONFIG])dnl
  min_gpgme_version=ifelse([$1], ,0.4.2,$1)
  AC_MSG_CHECKING(for GPGME Pth - version >= $min_gpgme_version)
  ok=no
  if test "$GPGME_CONFIG" != "no" ; then
    if `$GPGME_CONFIG --thread=pth 2> /dev/null` ; then
      req_major=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\1/'`
      req_minor=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\2/'`
      req_micro=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\3/'`
      if test "$gpgme_version_major" -gt "$req_major"; then
        ok=yes
      else 
        if test "$gpgme_version_major" -eq "$req_major"; then
          if test "$gpgme_version_minor" -gt "$req_minor"; then
            ok=yes
          else
            if test "$gpgme_version_minor" -eq "$req_minor"; then
              if test "$gpgme_version_micro" -ge "$req_micro"; then
                ok=yes
              fi
            fi
          fi
        fi
      fi
    fi
  fi
  if test $ok = yes; then
    GPGME_PTH_CFLAGS=`$GPGME_CONFIG --thread=pth --cflags`
    GPGME_PTH_LIBS=`$GPGME_CONFIG --thread=pth --libs`
    AC_MSG_RESULT(yes)
    ifelse([$2], , :, [$2])
  else
    GPGME_PTH_CFLAGS=""
    GPGME_PTH_LIBS=""
    AC_MSG_RESULT(no)
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GPGME_PTH_CFLAGS)
  AC_SUBST(GPGME_PTH_LIBS)
])

dnl AM_PATH_GPGME_PTHREAD([MINIMUM-VERSION,
dnl                       [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libgpgme and define GPGME_PTHREAD_CFLAGS
dnl  and GPGME_PTHREAD_LIBS.
dnl
AC_DEFUN([AM_PATH_GPGME_PTHREAD],
[ AC_REQUIRE([_AM_PATH_GPGME_CONFIG])dnl
  min_gpgme_version=ifelse([$1], ,0.4.2,$1)
  AC_MSG_CHECKING(for GPGME pthread - version >= $min_gpgme_version)
  ok=no
  if test "$GPGME_CONFIG" != "no" ; then
    if `$GPGME_CONFIG --thread=pthread 2> /dev/null` ; then
      req_major=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\1/'`
      req_minor=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\2/'`
      req_micro=`echo $min_gpgme_version | \
               sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\)/\3/'`
      if test "$gpgme_version_major" -gt "$req_major"; then
        ok=yes
      else 
        if test "$gpgme_version_major" -eq "$req_major"; then
          if test "$gpgme_version_minor" -gt "$req_minor"; then
            ok=yes
          else
            if test "$gpgme_version_minor" -eq "$req_minor"; then
              if test "$gpgme_version_micro" -ge "$req_micro"; then
                ok=yes
              fi
            fi
          fi
        fi
      fi
    fi
  fi
  if test $ok = yes; then
    GPGME_PTHREAD_CFLAGS=`$GPGME_CONFIG --thread=pthread --cflags`
    GPGME_PTHREAD_LIBS=`$GPGME_CONFIG --thread=pthread --libs`
    AC_MSG_RESULT(yes)
    ifelse([$2], , :, [$2])
  else
    GPGME_PTHREAD_CFLAGS=""
    GPGME_PTHREAD_LIBS=""
    AC_MSG_RESULT(no)
    ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GPGME_PTHREAD_CFLAGS)
  AC_SUBST(GPGME_PTHREAD_LIBS)
])
