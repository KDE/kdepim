/* Define if you have execinfo.h */
#cmakedefine HAVE_EXECINFO_H 1

/* Define if you have cyrus-sasl2 libraries */
#cmakedefine HAVE_LIBSASL2 1

/* Define if you have gpgme libraries */
#cmakedefine HAVE_GPGME

/* Define if you have the gethostname prototype */
#cmakedefine HAVE_GETHOSTNAME_PROTO 1

/* Define if you have the getdomainname prototype */
#cmakedefine HAVE_GETDOMAINNAME_PROTO 1

/* Define to 1 if you have the `setenv' function. */
#cmakedefine HAVE_SETENV 1

/* Define to 1 if you have the `setenv' function prototype. */
#cmakedefine HAVE_SETENV_PROTO 1

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <sys/cdefs.h> header file. */
#cmakedefine HAVE_SYS_CDEFS_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H 1

/* Define to 1 if you have the <sys/file.h> header file. */
#cmakedefine HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#cmakedefine HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/limits.h> header file. */
#cmakedefine HAVE_SYS_LIMITS_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#cmakedefine HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#cmakedefine HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#cmakedefine HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#cmakedefine HAVE_TIME_H 1

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#cmakedefine HAVE_UNSETENV 1

/* Define if you have the unsetenv prototype */
#cmakedefine HAVE_UNSETENV_PROTO 1

/* Define if you have usleep */
#cmakedefine HAVE_USLEEP 1

/* Define if you have the usleep prototype */
#cmakedefine HAVE_USLEEP_PROTO 1

/* Define if you have strlcat (BSD*) */
#cmakedefine HAVE_STRLCAT 1

/* Define if you have strlcat prototype (BSD*) */
#cmakedefine HAVE_STRLCAT_PROTO 1

/* Define if you have strlcpy (BSD*) */
#cmakedefine HAVE_STRLCPY 1

/* Define if you have strlcpy prototype (BSD*) */
#cmakedefine HAVE_STRLCPY_PROTO 1


/* Define to 1 if you have the <values.h> header file. */
#cmakedefine HAVE_VALUES_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine HAVE_VSNPRINTF 1


/* Define to 1 if you want to use the new distribution lists */
#cmakedefine KDEPIM_NEW_DISTRLISTS 1

/* Maximum length of command line arguments */
#define MAX_CMD_LENGTH 32768

/* Name of package */
#define PACKAGE "kdepim"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* The size of a `char *', as computed by sizeof. */
#define SIZEOF_CHAR_P ${SIZEOF_CHAR_P}

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT ${SIZEOF_INT}

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT ${SIZEOF_SHORT}

/* The size of a `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T ${SIZEOF_SIZE_T}

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG ${SIZEOF_UNSIGNED_LONG}


/* The size of a `uint64_t', as computed by sizeof. */
#define SIZEOF_UINT64_T ${SIZEOF_UINT64_T}

/* The size of a `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG ${SIZEOF_UNSIGNED_LONG_LONG}

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Namespace prefix the stl is defined in */
#define STD_NAMESPACE_PREFIX std::

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME 1

/* Version number of package */
#define VERSION "3.9.02"

/* Defined if compiling without arts */
#define WITHOUT_ARTS 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#define YYTEXT_POINTER 1

/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif



/*
 * AIX defines FD_SET in terms of bzero, but fails to include <strings.h>
 * that defines bzero.
 */

#if defined(_AIX)
#include <strings.h>
#endif



#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif


/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64



/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */


/*
 * On HP-UX, the declaration of vsnprintf() is needed every time !
 */

#if !defined(HAVE_VSNPRINTF) || defined(hpux)
#if __STDC__
#include <stdarg.h>
#include <stdlib.h>
#else
#include <varargs.h>
#endif
#ifdef __cplusplus
extern "C"
#endif
int vsnprintf(char *str, size_t n, char const *fmt, va_list ap);
#ifdef __cplusplus
extern "C"
#endif
int snprintf(char *str, size_t n, char const *fmt, ...);
#endif



#if defined(__SVR4) && !defined(__svr4__)
#define __svr4__ 1
#endif


/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* type to use in place of socklen_t if not defined */
#define kde_socklen_t socklen_t

/* type to use in place of socklen_t if not defined (deprecated, use
   kde_socklen_t) */
#define ksize_t socklen_t

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */
