/* Define to 1 if you want to use the new distribution lists */
#cmakedefine KDEPIM_NEW_DISTRLISTS 1

#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#else
#define KPATH_SEPARATOR ':'
#endif

