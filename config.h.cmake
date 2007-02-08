/* Define if you have cyrus-sasl2 libraries */
#cmakedefine HAVE_LIBSASL2 1

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

