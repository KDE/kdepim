/*
  Backward compatibility support.
  By policy, kdepim must compile against the most recent kdelibs minor release.
*/

/* Allow kdepim to build against kdelibs 4.0 */
#if !defined(KPATH_SEPARATOR)
#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#else
#define KPATH_SEPARATOR ':'
#endif
#endif
