/* Define if you have cyrus-sasl2 libraries */
#cmakedefine HAVE_LIBSASL2 1

/* Define if you have gpgme libraries */
#cmakedefine HAVE_GPGME

/* Define to 1 if you have the <sys/cdefs.h> header file. */
#cmakedefine HAVE_SYS_CDEFS_H 1

/* Define to 1 if you have the <sys/limits.h> header file. */
#cmakedefine HAVE_SYS_LIMITS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <values.h> header file. */
/* Needed by ./kresources/slox/webdavhandler.cpp only, to be moved */
#cmakedefine HAVE_VALUES_H 1

/* Define to 1 if you want to use the new distribution lists */
#cmakedefine KDEPIM_NEW_DISTRLISTS 1

/* Maximum length of command line arguments */
/* Needed by ./libkleo/backends/qgpgme/qgpgmerefreshkeysjob.cpp - to be moved there */
#define MAX_CMD_LENGTH 32768

/* The size of a `long', as computed by sizeof. */
/* Needed by akregator; to be moved to a config-akregator.h */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of a `unsigned long', as computed by sizeof. */
/* Needed by ./libksieve/tests/parsertest.cpp; to be moved */
#define SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME 1
