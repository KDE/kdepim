/* Define to 1 if you have a recent enough libassuan */
#cmakedefine HAVE_USABLE_ASSUAN 1

/* Define to 1 if you have libassuan v2 */
#cmakedefine HAVE_ASSUAN2 1

#ifndef HAVE_ASSUAN2
/* Define to 1 if your libassuan has the assuan_fd_t type  */
#cmakedefine HAVE_ASSUAN_FD_T 1

/* Define to 1 if your libassuan has the assuan_inquire_ext function */
#cmakedefine HAVE_ASSUAN_INQUIRE_EXT 1

/* Define to 1 if your assuan_inquire_ext puts the buffer arguments into the callback signature */
#cmakedefine HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT 1

/* Define to 1 if your libassuan has the assuan_sock_get_nonce function */
#cmakedefine HAVE_ASSUAN_SOCK_GET_NONCE 1

#endif
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
