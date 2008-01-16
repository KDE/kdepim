/* Define to 1 if you have a recent enough libassuan */
#cmakedefine HAVE_USABLE_ASSUAN 1

/* Define to 1 if your libassuan has the assuan_fd_t type  */
#cmakedefine HAVE_ASSUAN_FD_T 1

#ifndef HAVE_ASSUAN_FD_T
typedef int assuan_fd_t; // this doesn't work on Windows, but then HAVE_USABLE_ASSUAN isn't defined, either.
#endif // HAVE_ASSUAN_FD_T

/* Define to 1 if your libassuan has the assuan_inquire_ext function */
#cmakedefine HAVE_ASSUAN_INQUIRE_EXT 1

/* Define to 1 if your assuan_inquire_ext puts the buffer arguments into the callback signature */
#cmakedefine HAVE_NEW_STYLE_ASSUAN_INQUIRE_EXT 1

/* Define to 1 if your libassuan has the assuan_sock_get_nonce function */
#cmakedefine HAVE_ASSUAN_SOCK_GET_NONCE 1

#cmakedefine HAVE_ASSUAN 1

#define KLEOPATRA_VERSION_STRING "@kleopatra_version@"
