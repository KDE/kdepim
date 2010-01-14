/* Define to 1 if you have a recent enough libassuan */
#cmakedefine HAVE_USABLE_ASSUAN 1

/* Define to 1 if you build libkleopatraclient */
#cmakedefine HAVE_KLEOPATRACLIENT_LIBRARY 1

#define KLEOPATRA_VERSION_STRING "@kleopatra_version@"

#if defined _WIN32 || defined _WIN64
#include <kde_file_win.h>
#endif

#if !defined(KPATH_SEPARATOR)
#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#else
#define KPATH_SEPARATOR ':'
#endif
#endif
