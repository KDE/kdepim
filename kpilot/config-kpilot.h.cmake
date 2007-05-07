#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_ALLOCA_H
#cmakedefine HAVE_SYS_TIME_H
#cmakedefine HAVE_SYS_STAT_H
#cmakedefine HAVE_CFSETSPEED
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_SETENV
#cmakedefine HAVE_UNSETENV
#cmakedefine HAVE_USLEEP
#cmakedefine HAVE_RANDOM
#cmakedefine HAVE_PUTENV
#cmakedefine HAVE_SETEUID
#cmakedefine HAVE_MKSTEMPS
#cmakedefine HAVE_MKSTEMP
#cmakedefine HAVE_MKDTEMP
#cmakedefine HAVE_REVOKE
#cmakedefine HAVE_STRLCPY
#cmakedefine HAVE_STRLCAT
#cmakedefine HAVE_INET_ATON

#if !defined(HAVE_STRLCAT)
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcat(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif



#if !defined(HAVE_STRLCPY)
#ifdef __cplusplus
extern "C" {
#endif
unsigned long strlcpy(char*, const char*, unsigned long);
#ifdef __cplusplus
}
#endif
#endif


