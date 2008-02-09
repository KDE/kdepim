/* Define to 1 if you want to use the new distribution lists */
#cmakedefine KDEPIM_NEW_DISTRLISTS 1

/* define to 1 if you have sys/poll.h */
#cmakedefine HAVE_SYS_POLL_H

/* The size of a `long', as computed by sizeof. */
/* Needed by akregator; to be moved to a config-akregator.h */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of a `unsigned long', as computed by sizeof. */
/* Needed by ./libksieve/tests/parsertest.cpp; to be moved */
#define SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}

#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#else
#define KPATH_SEPARATOR ':'
#endif

