#ifndef KCALENDARSYSTEM_HCONV_H
#define KCALENDARSYSTEM_HCONV_H

extern char	*mname[], *hmname[], *dow[], *sdow[];

typedef struct sdate {
	float time;
	int day;
	int mon;
	int year;
	int dw;
	double nmtime;
} SDATE;

#ifdef __cplusplus
	extern "C" {
#endif
extern SDATE *caldate(double), *hdate(int,int,int), *gdate(int,int,int);

extern double tmoonphase(long,int), julianday(int,int,int,float);
extern int ndays(int,int);
#ifdef __cplusplus
	}
#endif

#define NULLP    ((char *)0)

/* for portability */
#include <sys/types.h>

#endif
