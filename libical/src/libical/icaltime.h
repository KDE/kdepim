/* -*- Mode: C -*- */
/*======================================================================
 FILE: icaltime.h
 CREATOR: eric 02 June 2000


 $Id$
 $Locker:  $

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Original Code is eric. The Initial Developer of the Original
 Code is Eric Busboom


======================================================================*/

#ifndef ICALTIME_H
#define ICALTIME_H

#include <time.h>

/* icaltime_span is returned by icalcomponent_get_span() */
struct icaltime_span {
	time_t start; /* in UTC */
	time_t end; /* in UTC */
	int is_busy; /* 1->busy time, 0-> free time */
};

struct icaltimetype
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	int is_utc; /* 1-> time is in UTC timezone */

	int is_date; /* 1 -> interpret this as date. */
};	

struct icaltimetype icaltime_null_time(void);

int icaltime_is_null_time(struct icaltimetype t);

struct icaltimetype icaltime_normalize(struct icaltimetype t);

short icaltime_day_of_year(struct icaltimetype t);
struct icaltimetype icaltime_from_day_of_year(short doy,  short year);

short icaltime_day_of_week(struct icaltimetype t);
short icaltime_start_doy_of_week(struct icaltimetype t);

struct icaltimetype icaltime_from_timet(time_t v, int is_date, int is_utc);
struct icaltimetype icaltime_from_string(const char* str);
time_t icaltime_as_timet(struct icaltimetype);
char* icaltime_as_ctime(struct icaltimetype);

short icaltime_week_number(short day_of_month, short month, short year);

struct icaltimetype icaltime_from_week_number(short week_number, short year);

int icaltime_compare(struct icaltimetype a,struct icaltimetype b);


short icaltime_days_in_month(short month,short year);

/* Routines for handling timezones */

/* Return the offset of the named zone as seconds. tt is a time
   indicating the date for which you want the offset */
time_t icaltime_utc_offset(struct icaltimetype tt, const char* tzid);

time_t icaltime_local_utc_offset();


/* convert tt, of timezone tzid, into a utc time */
struct icaltimetype icaltime_as_utc(struct icaltimetype tt,const char* tzid);

/* convert tt, a time in UTC, into a time in timezone tzid */
struct icaltimetype icaltime_as_zone(struct icaltimetype tt,const char* tzid);



struct icaldurationtype
{
	int is_neg;
	unsigned int days;
	unsigned int weeks;
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;
};

struct icaldurationtype icaldurationtype_from_timet(time_t t);
struct icaldurationtype icaldurationtype_from_string(const char*);
time_t icaldurationtype_as_timet(struct icaldurationtype duration);


struct icalperiodtype 
{
	struct icaltimetype start; /* Must be absolute */	
	struct icaltimetype end; /* Must be absolute */
	struct icaldurationtype duration;
};

time_t icalperiodtype_duration(struct icalperiodtype period);
time_t icalperiodtype_end(struct icalperiodtype period);



struct icaltimetype  icaltime_add(struct icaltimetype t,
				  struct icaldurationtype  d);

struct icaldurationtype  icaltime_subtract(struct icaltimetype t1,
					   struct icaltimetype t2);


#endif /* !ICALTIME_H */



