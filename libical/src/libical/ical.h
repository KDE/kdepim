#ifndef ICAL_VERSION_H
#define ICAL_VERSION_H

#define ICAL_PACKAGE "libical"
#define ICAL_VERSION "0.21"

#endif
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




/* -*- Mode: C -*-*/
/*======================================================================
 FILE: icalenums.h

 

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalenums.h

  Contributions from:
     Graham Davison (g.m.davison@computer.org)

======================================================================*/

#ifndef ICALENUMS_H
#define ICALENUMS_H



/***********************************************************************
 * Component enumerations
**********************************************************************/

typedef enum icalcomponent_kind {
    ICAL_NO_COMPONENT,
    ICAL_ANY_COMPONENT,	/* Used to select all components*/
    ICAL_XROOT_COMPONENT,
    ICAL_XATTACH_COMPONENT, /* MIME attached data, returned by parser. */
    ICAL_VEVENT_COMPONENT,
    ICAL_VTODO_COMPONENT,
    ICAL_VJOURNAL_COMPONENT,
    ICAL_VCALENDAR_COMPONENT,
    ICAL_VFREEBUSY_COMPONENT,
    ICAL_VALARM_COMPONENT,
    ICAL_XAUDIOALARM_COMPONENT,  
    ICAL_XDISPLAYALARM_COMPONENT,
    ICAL_XEMAILALARM_COMPONENT,
    ICAL_XPROCEDUREALARM_COMPONENT,
    ICAL_VTIMEZONE_COMPONENT,
    ICAL_XSTANDARD_COMPONENT,
    ICAL_XDAYLIGHT_COMPONENT,
    ICAL_X_COMPONENT,
    ICAL_VSCHEDULE_COMPONENT,
    ICAL_VQUERY_COMPONENT,
    ICAL_VCAR_COMPONENT,
    ICAL_VCOMMAND_COMPONENT,
    ICAL_XLICINVALID_COMPONENT,
    ICAL_XLICMIMEPART_COMPONENT /* a non-stardard component that mirrors
				structure of MIME data */

} icalcomponent_kind;

/***********************************************************************
 * Property Enumerations
**********************************************************************/

typedef enum icalproperty_kind {
    ICAL_ANY_PROPERTY = 0, /* This must be the first enum, for iteration */
    ICAL_CALSCALE_PROPERTY,
    ICAL_METHOD_PROPERTY,
    ICAL_PRODID_PROPERTY,
    ICAL_VERSION_PROPERTY,
    ICAL_ATTACH_PROPERTY,
    ICAL_CATEGORIES_PROPERTY,
    ICAL_CLASS_PROPERTY,
    ICAL_COMMENT_PROPERTY,
    ICAL_DESCRIPTION_PROPERTY,
    ICAL_GEO_PROPERTY,
    ICAL_LOCATION_PROPERTY,
    ICAL_PERCENTCOMPLETE_PROPERTY,
    ICAL_PRIORITY_PROPERTY,
    ICAL_RESOURCES_PROPERTY,
    ICAL_STATUS_PROPERTY,
    ICAL_SUMMARY_PROPERTY,
    ICAL_COMPLETED_PROPERTY,
    ICAL_DTEND_PROPERTY,
    ICAL_DUE_PROPERTY,
    ICAL_DTSTART_PROPERTY,
    ICAL_DURATION_PROPERTY,
    ICAL_FREEBUSY_PROPERTY,
    ICAL_TRANSP_PROPERTY,
    ICAL_TZID_PROPERTY,
    ICAL_TZNAME_PROPERTY,
    ICAL_TZOFFSETFROM_PROPERTY,
    ICAL_TZOFFSETTO_PROPERTY,
    ICAL_TZURL_PROPERTY,
    ICAL_ATTENDEE_PROPERTY,
    ICAL_CONTACT_PROPERTY,
    ICAL_ORGANIZER_PROPERTY,
    ICAL_RECURRENCEID_PROPERTY,
    ICAL_RELATEDTO_PROPERTY,
    ICAL_URL_PROPERTY,
    ICAL_UID_PROPERTY,
    ICAL_EXDATE_PROPERTY,
    ICAL_EXRULE_PROPERTY,
    ICAL_RDATE_PROPERTY,
    ICAL_RRULE_PROPERTY,
    ICAL_ACTION_PROPERTY,
    ICAL_REPEAT_PROPERTY,
    ICAL_TRIGGER_PROPERTY,
    ICAL_CREATED_PROPERTY,
    ICAL_DTSTAMP_PROPERTY,
    ICAL_LASTMODIFIED_PROPERTY,
    ICAL_SEQUENCE_PROPERTY,
    ICAL_REQUESTSTATUS_PROPERTY,
    ICAL_X_PROPERTY,

    /* CAP Properties */
    ICAL_SCOPE_PROPERTY,
    ICAL_MAXRESULTS_PROPERTY,
    ICAL_MAXRESULTSSIZE_PROPERTY,
    ICAL_QUERY_PROPERTY,
    ICAL_QUERYNAME_PROPERTY, 
    ICAL_TARGET_PROPERTY,

    /* libical private properties */
    ICAL_XLICERROR_PROPERTY,
    ICAL_XLICCLUSTERCOUNT_PROPERTY,
    ICAL_XLICMIMECONTENTTYPE_PROPERTY,
    ICAL_XLICMIMEENCODING_PROPERTY,
    ICAL_XLICMIMECID_PROPERTY,
    ICAL_XLICMIMEFILENAME_PROPERTY,
    ICAL_XLICMIMECHARSET_PROPERTY,
    ICAL_XLICMIMEOPTINFO_PROPERTY,

    ICAL_NO_PROPERTY /* This must be the last enum, for iteration */

} icalproperty_kind;

/***********************************************************************
 * Enumerations for the values of properties
 ***********************************************************************/

typedef enum icalproperty_method {
    ICAL_METHOD_PUBLISH,
    ICAL_METHOD_REQUEST,
    ICAL_METHOD_REPLY,
    ICAL_METHOD_ADD,
    ICAL_METHOD_CANCEL,
    ICAL_METHOD_REFRESH,
    ICAL_METHOD_COUNTER,
    ICAL_METHOD_DECLINECOUNTER,
    /* CAP Methods */
    ICAL_METHOD_CREATE,
    ICAL_METHOD_READ,
    ICAL_METHOD_RESPONSE,
    ICAL_METHOD_MOVE,
    ICAL_METHOD_MODIFY,
    ICAL_METHOD_GENERATEUID,
    ICAL_METHOD_DELETE,
    ICAL_METHOD_NONE
} icalproperty_method ;

typedef enum icalproperty_transp {
    ICAL_TRANSP_OPAQUE,
    ICAL_TRANS_TRANSPARENT
}  icalproperty_trans;

typedef enum icalproperty_calscale {
    ICAL_CALSCALE_GREGORIAN
} icalproperty_calscale ;


typedef enum icalproperty_class {
    ICAL_CLASS_PUBLIC,
    ICAL_CLASS_PRIVATE,
    ICAL_CLASS_CONFIDENTIAL,
    ICAL_CLASS_XNAME
} icalproperty_class;


typedef enum icalproperty_status {
    ICAL_STATUS_NONE,
    ICAL_STATUS_TENTATIVE,
    ICAL_STATUS_CONFIRMED,
    ICAL_STATUS_CANCELLED, /* CANCELED? SIC from RFC*/
    ICAL_STATUS_NEEDSACTION,
    ICAL_STATUS_COMPLETED,
    ICAL_STATUS_INPROCESS,
    ICAL_STATUS_DRAFT,
    ICAL_STATUS_FINAL
}  icalproperty_status;

typedef enum icalproperty_action {
    ICAL_ACTION_AUDIO,
    ICAL_ACTION_DISPLAY,
    ICAL_ACTION_EMAIL,
    ICAL_ACTION_PROCEDURE,
    ICAL_ACTION_XNAME
} icalproperty_action;

/***********************************************************************
 * Value enumerations
**********************************************************************/

typedef enum icalvalue_kind {
    ICAL_NO_VALUE,
    ICAL_ATTACH_VALUE, /* Non-Standard*/
    ICAL_BINARY_VALUE,
    ICAL_BOOLEAN_VALUE,
    ICAL_CALADDRESS_VALUE,
    ICAL_DATE_VALUE,
    ICAL_DATETIME_VALUE,
    ICAL_DATETIMEDATE_VALUE, /* Non-Standard */
    ICAL_DATETIMEPERIOD_VALUE, /* Non-Standard */
    ICAL_DURATION_VALUE,
    ICAL_FLOAT_VALUE,
    ICAL_GEO_VALUE, /* Non-Standard */
    ICAL_INTEGER_VALUE,
    ICAL_METHOD_VALUE, /* Non-Standard */
    ICAL_STATUS_VALUE, /* Non-Standard */
    ICAL_PERIOD_VALUE,
    ICAL_RECUR_VALUE,
    ICAL_STRING_VALUE, /* Non-Standard */
    ICAL_TEXT_VALUE,
    ICAL_TIME_VALUE,
    ICAL_TRIGGER_VALUE, /* Non-Standard */
    ICAL_URI_VALUE,
    ICAL_UTCOFFSET_VALUE,
    ICAL_QUERY_VALUE,
    ICAL_XNAME_VALUE
} icalvalue_kind;


/***********************************************************************
 * Parameter Enumerations
 **********************************************************************/


typedef enum icalparameter_kind {
    ICAL_NO_PARAMETER,
    ICAL_ANY_PARAMETER,
    ICAL_ALTREP_PARAMETER, /* DQUOTE uri DQUOTE */
    ICAL_CN_PARAMETER, /* text */
    ICAL_CUTYPE_PARAMETER, /*INDIVIDUAL, GROUP, RESOURCE,ROOM,UNKNOWN, x-name*/
    ICAL_DELEGATEDFROM_PARAMETER, /* *("," DQUOTE cal-address DQUOTE) */
    ICAL_DELEGATEDTO_PARAMETER, /*  *("," DQUOTE cal-address DQUOTE) */
    ICAL_DIR_PARAMETER, /*  DQUOTE uri DQUOTE */
    ICAL_ENCODING_PARAMETER, /* *BIT, BASE64, x-name */
    ICAL_FMTTYPE_PARAMETER, /* registered MINE content type */
    ICAL_FBTYPE_PARAMETER, /* FREE, BUSY, BUSY-UNAVAILABLE, BUSY-TENTATIVE,x-name */
    ICAL_LANGUAGE_PARAMETER, /* text from RFC 1766 */
    ICAL_MEMBER_PARAMETER, /*  DQUOTE cal-address DQUOTE */
    ICAL_PARTSTAT_PARAMETER, /* NEEDS-ACTION, ACCEPTED, DECLINED, TENTATIVE, DELEGATED, x-name */
    ICAL_RANGE_PARAMETER, /* THISANDPRIOR, THISANDFUTURE */
    ICAL_RELATED_PARAMETER, /* START, END */
    ICAL_RELTYPE_PARAMETER, /* PARENT, CHILD, SIBLING,x-name */
    ICAL_ROLE_PARAMETER, /* CHAIR, REQ_PARTICIPANT, OPT_PARTICIPANT, NON_PARTICIPANT, x-name */
    ICAL_RSVP_PARAMETER, /* TRUE. FALSE */
    ICAL_SENTBY_PARAMETER, /*  DQUOTE uri DQUOTE */
    ICAL_TZID_PARAMETER, /*  [tzidprefix] paramtext CRLF */
    ICAL_VALUE_PARAMETER, /* BINARY, BOOLEAN, CAL_ADDRESS, DATE, DATE-TIME, DURATION, FLOAT, INTEGER, PERIOD, RECUR, TEXT, TIME, UTC_OFFSET, x-name */
    ICAL_XLICERRORTYPE_PARAMETER, /*ICAL_XLICERROR_PARSE_ERROR,ICAL_XLICERROR_INVALID_ITIP*/
    ICAL_XLICCOMPARETYPE_PARAMETER, /**/
    ICAL_X_PARAMETER /* text */ 
} icalparameter_kind;

typedef enum icalparameter_cutype {
    ICAL_CUTYPE_INDIVIDUAL, 
    ICAL_CUTYPE_GROUP, 
    ICAL_CUTYPE_RESOURCE, 
    ICAL_CUTYPE_ROOM,
    ICAL_CUTYPE_UNKNOWN,
    ICAL_CUTYPE_XNAME
} icalparameter_cutype;


typedef enum icalparameter_encoding {
    ICAL_ENCODING_8BIT, 
    ICAL_ENCODING_BASE64,
    ICAL_ENCODING_XNAME
} icalparameter_encoding;

typedef enum icalparameter_fbtype {
    ICAL_FBTYPE_FREE, 
    ICAL_FBTYPE_BUSY, 
    ICAL_FBTYPE_BUSYUNAVAILABLE, 
    ICAL_FBTYPE_BUSYTENTATIVE,
    ICAL_FBTYPE_XNAME
} icalparameter_fbtype;

typedef enum icalparameter_partstat {
    ICAL_PARTSTAT_NEEDSACTION, 
    ICAL_PARTSTAT_ACCEPTED, 
    ICAL_PARTSTAT_DECLINED, 
    ICAL_PARTSTAT_TENTATIVE, 
    ICAL_PARTSTAT_DELEGATED,
    ICAL_PARTSTAT_COMPLETED,
    ICAL_PARTSTAT_INPROCESS,
    ICAL_PARTSTAT_XNAME,
    ICAL_PARTSTAT_NONE
} icalparameter_partstat;

typedef enum icalparameter_range {
    ICAL_RANGE_THISANDPRIOR, 
    ICAL_RANGE_THISANDFUTURE
} icalparameter_range;

typedef enum icalparameter_related {
    ICAL_RELATED_START, 
    ICAL_RELATED_END
} icalparameter_related;

typedef enum icalparameter_reltype {
    ICAL_RELTYPE_PARENT, 
    ICAL_RELTYPE_CHILD,
    ICAL_RELTYPE_SIBLING,
    ICAL_RELTYPE_XNAME
} icalparameter_reltype;

typedef enum icalparameter_role {
    ICAL_ROLE_CHAIR, 
    ICAL_ROLE_REQPARTICIPANT, 
    ICAL_ROLE_OPTPARTICIPANT, 
    ICAL_ROLE_NONPARTICIPANT,
    ICAL_ROLE_XNAME
} icalparameter_role;

typedef enum icalparameter_xlicerrortype {
    ICAL_XLICERRORTYPE_COMPONENTPARSEERROR,
    ICAL_XLICERRORTYPE_PARAMETERVALUEPARSEERROR,
    ICAL_XLICERRORTYPE_PARAMETERNAMEPARSEERROR,
    ICAL_XLICERRORTYPE_PROPERTYPARSEERROR,
    ICAL_XLICERRORTYPE_VALUEPARSEERROR,
    ICAL_XLICERRORTYPE_UNKVCALPROP,
    ICAL_XLICERRORTYPE_INVALIDITIP,
    ICAL_XLICERRORTYPE_MIMEPARSEERROR
} icalparameter_xlicerrortype;

typedef enum icalparameter_xliccomparetype {
    ICAL_XLICCOMPARETYPE_EQUAL=0,
    ICAL_XLICCOMPARETYPE_LESS=-1,
    ICAL_XLICCOMPARETYPE_LESSEQUAL=2,
    ICAL_XLICCOMPARETYPE_GREATER=1,
    ICAL_XLICCOMPARETYPE_GREATEREQUAL=3,
    ICAL_XLICCOMPARETYPE_NOTEQUAL=4,
    ICAL_XLICCOMPARETYPE_REGEX=5
} icalparameter_xliccomparetype;

typedef enum icalparameter_value {
    ICAL_VALUE_XNAME = ICAL_XNAME_VALUE,
    ICAL_VALUE_BINARY = ICAL_BINARY_VALUE, 
    ICAL_VALUE_BOOLEAN = ICAL_BOOLEAN_VALUE, 
    ICAL_VALUE_CALADDRESS = ICAL_CALADDRESS_VALUE, 
    ICAL_VALUE_DATE = ICAL_DATE_VALUE, 
    ICAL_VALUE_DATETIME = ICAL_DATETIME_VALUE, 
    ICAL_VALUE_DURATION = ICAL_DURATION_VALUE, 
    ICAL_VALUE_FLOAT = ICAL_FLOAT_VALUE, 
    ICAL_VALUE_INTEGER = ICAL_INTEGER_VALUE, 
    ICAL_VALUE_PERIOD = ICAL_PERIOD_VALUE, 
    ICAL_VALUE_RECUR = ICAL_RECUR_VALUE, 
    ICAL_VALUE_TEXT = ICAL_TEXT_VALUE, 
    ICAL_VALUE_TIME = ICAL_TIME_VALUE, 
    ICAL_VALUE_UTCOFFSET = ICAL_UTCOFFSET_VALUE,
    ICAL_VALUE_URI = ICAL_URI_VALUE,
    ICAL_VALUE_ERROR = ICAL_NO_VALUE
} icalparameter_value;

/***********************************************************************
 * Recurrances 
**********************************************************************/

typedef enum icalrecurrencetype_frequency
{
    /* These enums are used to index an array, so don't change the
       order or the integers */

    ICAL_SECONDLY_RECURRENCE=0,
    ICAL_MINUTELY_RECURRENCE=1,
    ICAL_HOURLY_RECURRENCE=2,
    ICAL_DAILY_RECURRENCE=3,
    ICAL_WEEKLY_RECURRENCE=4,
    ICAL_MONTHLY_RECURRENCE=5,
    ICAL_YEARLY_RECURRENCE=6,
    ICAL_NO_RECURRENCE=7

} icalrecurrencetype_frequency;

typedef enum icalrecurrencetype_weekday
{
    ICAL_NO_WEEKDAY,
    ICAL_SUNDAY_WEEKDAY,
    ICAL_MONDAY_WEEKDAY,
    ICAL_TUESDAY_WEEKDAY,
    ICAL_WEDNESDAY_WEEKDAY,
    ICAL_THURSDAY_WEEKDAY,
    ICAL_FRIDAY_WEEKDAY,
    ICAL_SATURDAY_WEEKDAY
} icalrecurrencetype_weekday;

enum {
    ICAL_RECURRENCE_ARRAY_MAX = 0x7f7f,
    ICAL_RECURRENCE_ARRAY_MAX_BYTE = 0x7f
};
    

const char* icalenum_recurrence_to_string(icalrecurrencetype_frequency kind);
const char* icalenum_weekday_to_string(icalrecurrencetype_weekday kind);

/***********************************************************************
 * Request Status codes
 **********************************************************************/

typedef enum icalrequeststatus {
    ICAL_UNKNOWN_STATUS,
    ICAL_2_0_SUCCESS_STATUS,
    ICAL_2_1_FALLBACK_STATUS,
    ICAL_2_2_IGPROP_STATUS,
    ICAL_2_3_IGPARAM_STATUS,
    ICAL_2_4_IGXPROP_STATUS,
    ICAL_2_5_IGXPARAM_STATUS,
    ICAL_2_6_IGCOMP_STATUS,
    ICAL_2_7_FORWARD_STATUS,
    ICAL_2_8_ONEEVENT_STATUS,
    ICAL_2_9_TRUNC_STATUS,
    ICAL_2_10_ONETODO_STATUS,
    ICAL_2_11_TRUNCRRULE_STATUS,
    ICAL_3_0_INVPROPNAME_STATUS,
    ICAL_3_1_INVPROPVAL_STATUS,
    ICAL_3_2_INVPARAM_STATUS,
    ICAL_3_3_INVPARAMVAL_STATUS,
    ICAL_3_4_INVCOMP_STATUS,
    ICAL_3_5_INVTIME_STATUS,
    ICAL_3_6_INVRULE_STATUS,
    ICAL_3_7_INVCU_STATUS,
    ICAL_3_8_NOAUTH_STATUS,
    ICAL_3_9_BADVERSION_STATUS,
    ICAL_3_10_TOOBIG_STATUS,
    ICAL_3_11_MISSREQCOMP_STATUS,
    ICAL_3_12_UNKCOMP_STATUS,
    ICAL_3_13_BADCOMP_STATUS,
    ICAL_3_14_NOCAP_STATUS,
    ICAL_4_0_BUSY_STATUS,
    ICAL_5_0_MAYBE_STATUS,
    ICAL_5_1_UNAVAIL_STATUS,
    ICAL_5_2_NOSERVICE_STATUS,
    ICAL_5_3_NOSCHED_STATUS
} icalrequeststatus;


const char* icalenum_reqstat_desc(icalrequeststatus stat);
short icalenum_reqstat_major(icalrequeststatus stat);
short icalenum_reqstat_minor(icalrequeststatus stat);
icalrequeststatus icalenum_num_to_reqstat(short major, short minor);

/***********************************************************************
 * Conversion functions
**********************************************************************/

const char* icalenum_property_kind_to_string(icalproperty_kind kind);
icalproperty_kind icalenum_string_to_property_kind(char* string);

const char* icalenum_value_kind_to_string(icalvalue_kind kind);
icalvalue_kind icalenum_value_kind_by_prop(icalproperty_kind kind);

const char* icalenum_parameter_kind_to_string(icalparameter_kind kind);
icalparameter_kind icalenum_string_to_parameter_kind(char* string);

const char* icalenum_component_kind_to_string(icalcomponent_kind kind);
icalcomponent_kind icalenum_string_to_component_kind(char* string);

icalvalue_kind icalenum_property_kind_to_value_kind(icalproperty_kind kind);

const char* icalenum_method_to_string(icalproperty_method);
icalproperty_method icalenum_string_to_method(const char* string);

const char* icalenum_status_to_string(icalproperty_status);
icalproperty_status icalenum_string_to_status(const char* string);

#endif /* !ICALENUMS_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icaltypes.h
 CREATOR: eric 20 March 1999


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icaltypes.h

======================================================================*/

#ifndef ICALTYPES_H
#define ICALTYPES_H

#include <time.h>

/* This type type should probably be an opaque type... */
struct icalattachtype
{
	void* binary;
	int owns_binary; 

	char* base64;
	int owns_base64;

	char* url;

	int refcount; 

};

/* converts base64 to binary, fetches url and stores as binary, or
   just returns data */

struct icalattachtype* icalattachtype_new(void);
void  icalattachtype_add_reference(struct icalattachtype* v);
void icalattachtype_free(struct icalattachtype* v);

void icalattachtype_set_url(struct icalattachtype* v, char* url);
char* icalattachtype_get_url(struct icalattachtype* v);

void icalattachtype_set_base64(struct icalattachtype* v, char* base64,
				int owns);
char* icalattachtype_get_base64(struct icalattachtype* v);

void icalattachtype_set_binary(struct icalattachtype* v, char* binary,
				int owns);
void* icalattachtype_get_binary(struct icalattachtype* v);

struct icalgeotype 
{
	float lat;
	float lon;
};

					   

/* See RFC 2445 Section 4.3.10, RECUR Value, for an explaination of
   the values and fields in struct icalrecurrencetype */


struct icalrecurrencetype 
{
	icalrecurrencetype_frequency freq;


	/* until and count are mutually exclusive. */
       	struct icaltimetype until;
	int count;

	short interval;
	
	icalrecurrencetype_weekday week_start;
	
	/* The BY* parameters can each take a list of values. Here I
	 * assume that the list of values will not be larger than the
	 * range of the value -- that is, the client will not name a
	 * value more than once. 
	 
	 * Each of the lists is terminated with the value SHRT_MAX
	 * unless the the list is full. */

	short by_second[61];
	short by_minute[61];
	short by_hour[25];
	short by_day[8]; /* Encoded value, see below */
	short by_month_day[32];
	short by_year_day[367];
	short by_week_no[54];
	short by_month[13];
	short by_set_pos[367];
};


void icalrecurrencetype_clear(struct icalrecurrencetype *r);

/* The 'day' element of icalrecurrencetype_weekday is encoded to allow
reporesentation of both the day of the week ( Monday, Tueday), but
also the Nth day of the week ( First tuesday of the month, last
thursday of the year) These routines decode the day values */

/* 1 == Monday, etc. */
enum icalrecurrencetype_weekday icalrecurrencetype_day_day_of_week(short day);

/* 0 == any of day of week. 1 == first, 2 = second, -2 == second to last, etc */
short icalrecurrencetype_day_position(short day);

/* Return the next occurance of 'r' after the time specified by 'after' */
struct icaltimetype icalrecurrencetype_next_occurance(
    struct icalrecurrencetype *r,
    struct icaltimetype *after);

union icaltriggertype 
{
	struct icaltimetype time; 
	struct icaldurationtype duration;
};



/* struct icalreqstattype. This struct contains two string pointers,
but don't try to free either of them. The "desc" string is a pointer
to a static table inside the library.  Don't try to free it. The
"debug" string is a pointer into the string that the called passed
into to icalreqstattype_from_string. Don't try to free it either, and
don't use it after the original string has been freed.

BTW, you would get that original string from
*icalproperty_get_requeststatus() or icalvalue_get_text(), when
operating on a the value of a request_status property. */

struct icalreqstattype {

	icalrequeststatus code;
	const char* desc;
	const char* debug;
};

struct icalreqstattype icalreqstattype_from_string(char* str);
char* icalreqstattype_as_string(struct icalreqstattype);

#endif /* !ICALTYPES_H */
/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalvalue.h
  CREATOR: eric 20 March 1999


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

  The original code is icalvalue.h

  ======================================================================*/

#ifndef ICALVALUE_H
#define ICALVALUE_H

#include <time.h>
                          
typedef void icalvalue;

icalvalue* icalvalue_new(icalvalue_kind kind);

icalvalue* icalvalue_new_clone(icalvalue* value);

icalvalue* icalvalue_new_from_string(icalvalue_kind kind, char* str);

void icalvalue_free(icalvalue* value);

int icalvalue_is_valid(icalvalue* value);

const char* icalvalue_as_ical_string(icalvalue* value);

icalvalue_kind icalvalue_isa(icalvalue* value);

int icalvalue_isa_value(void*);

icalparameter_xliccomparetype
icalvalue_compare(icalvalue* a, icalvalue *b);

/* Everything below this line is machine generated. Do not edit. */
/* ATTACH # Non-std */
icalvalue* icalvalue_new_attach(struct icalattachtype v);
struct icalattachtype icalvalue_get_attach(icalvalue* value);
void icalvalue_set_attach(icalvalue* value, struct icalattachtype v);

/* BINARY  */
icalvalue* icalvalue_new_binary(const char* v);
const char* icalvalue_get_binary(icalvalue* value);
void icalvalue_set_binary(icalvalue* value, const char* v);

/* BOOLEAN  */
icalvalue* icalvalue_new_boolean(int v);
int icalvalue_get_boolean(icalvalue* value);
void icalvalue_set_boolean(icalvalue* value, int v);

/* CAL-ADDRESS  */
icalvalue* icalvalue_new_caladdress(const char* v);
const char* icalvalue_get_caladdress(icalvalue* value);
void icalvalue_set_caladdress(icalvalue* value, const char* v);

/* DATE  */
icalvalue* icalvalue_new_date(struct icaltimetype v);
struct icaltimetype icalvalue_get_date(icalvalue* value);
void icalvalue_set_date(icalvalue* value, struct icaltimetype v);

/* DATE-TIME  */
icalvalue* icalvalue_new_datetime(struct icaltimetype v);
struct icaltimetype icalvalue_get_datetime(icalvalue* value);
void icalvalue_set_datetime(icalvalue* value, struct icaltimetype v);

/* DATE-TIME-DATE # Non-std */
icalvalue* icalvalue_new_datetimedate(struct icaltimetype v);
struct icaltimetype icalvalue_get_datetimedate(icalvalue* value);
void icalvalue_set_datetimedate(icalvalue* value, struct icaltimetype v);

/* DATE-TIME-PERIOD # Non-std */
icalvalue* icalvalue_new_datetimeperiod(struct icalperiodtype v);
struct icalperiodtype icalvalue_get_datetimeperiod(icalvalue* value);
void icalvalue_set_datetimeperiod(icalvalue* value, struct icalperiodtype v);

/* DURATION  */
icalvalue* icalvalue_new_duration(struct icaldurationtype v);
struct icaldurationtype icalvalue_get_duration(icalvalue* value);
void icalvalue_set_duration(icalvalue* value, struct icaldurationtype v);

/* FLOAT  */
icalvalue* icalvalue_new_float(float v);
float icalvalue_get_float(icalvalue* value);
void icalvalue_set_float(icalvalue* value, float v);

/* GEO # Non-std */
icalvalue* icalvalue_new_geo(struct icalgeotype v);
struct icalgeotype icalvalue_get_geo(icalvalue* value);
void icalvalue_set_geo(icalvalue* value, struct icalgeotype v);

/* INTEGER  */
icalvalue* icalvalue_new_integer(int v);
int icalvalue_get_integer(icalvalue* value);
void icalvalue_set_integer(icalvalue* value, int v);

/* METHOD # Non-std */
icalvalue* icalvalue_new_method(icalproperty_method v);
icalproperty_method icalvalue_get_method(icalvalue* value);
void icalvalue_set_method(icalvalue* value, icalproperty_method v);

/* PERIOD  */
icalvalue* icalvalue_new_period(struct icalperiodtype v);
struct icalperiodtype icalvalue_get_period(icalvalue* value);
void icalvalue_set_period(icalvalue* value, struct icalperiodtype v);

/* RECUR  */
icalvalue* icalvalue_new_recur(struct icalrecurrencetype v);
struct icalrecurrencetype icalvalue_get_recur(icalvalue* value);
void icalvalue_set_recur(icalvalue* value, struct icalrecurrencetype v);

/* STRING # Non-std */
icalvalue* icalvalue_new_string(const char* v);
const char* icalvalue_get_string(icalvalue* value);
void icalvalue_set_string(icalvalue* value, const char* v);

/* TEXT  */
icalvalue* icalvalue_new_text(const char* v);
const char* icalvalue_get_text(icalvalue* value);
void icalvalue_set_text(icalvalue* value, const char* v);

/* TIME  */
icalvalue* icalvalue_new_time(struct icaltimetype v);
struct icaltimetype icalvalue_get_time(icalvalue* value);
void icalvalue_set_time(icalvalue* value, struct icaltimetype v);

/* TRIGGER # Non-std */
icalvalue* icalvalue_new_trigger(union icaltriggertype v);
union icaltriggertype icalvalue_get_trigger(icalvalue* value);
void icalvalue_set_trigger(icalvalue* value, union icaltriggertype v);

/* URI  */
icalvalue* icalvalue_new_uri(const char* v);
const char* icalvalue_get_uri(icalvalue* value);
void icalvalue_set_uri(icalvalue* value, const char* v);

/* UTC-OFFSET  */
icalvalue* icalvalue_new_utcoffset(int v);
int icalvalue_get_utcoffset(icalvalue* value);
void icalvalue_set_utcoffset(icalvalue* value, int v);

/* QUERY  */
icalvalue* icalvalue_new_query(const char* v);
const char* icalvalue_get_query(icalvalue* value);
void icalvalue_set_query(icalvalue* value, const char* v);

/* STATUS #Non-st */
icalvalue* icalvalue_new_status(icalproperty_status v);
icalproperty_status icalvalue_get_status(icalvalue* value);
void icalvalue_set_status(icalvalue* value, icalproperty_status v);

#endif /*ICALVALUE_H*/
/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalparam.h
  CREATOR: eric 20 March 1999


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

  The original code is icalparam.h

  ======================================================================*/

#ifndef ICALPARAM_H
#define ICALPARAM_H


typedef void icalparameter;

icalparameter* icalparameter_new(icalparameter_kind kind);
icalparameter* icalparameter_new_clone(icalparameter* p);
icalparameter* icalparameter_new_from_string(icalparameter_kind kind, char* value);

void icalparameter_free(icalparameter* parameter);

char* icalparameter_as_ical_string(icalparameter* parameter);

int icalparameter_is_valid(icalparameter* parameter);

icalparameter_kind icalparameter_isa(icalparameter* parameter);

int icalparameter_isa_parameter(void* param);

/* Acess the name of an X parameer */
void icalparameter_set_xname (icalparameter* param, const char* v);
const char* icalparameter_get_xname(icalparameter* param);
void icalparameter_set_xvalue (icalparameter* param, const char* v);
const char* icalparameter_get_xvalue(icalparameter* param);


/* Everything below this line is machine generated. Do not edit. */
/* ALTREP */
icalparameter* icalparameter_new_altrep(const char* v);
const char* icalparameter_get_altrep(icalparameter* value);
void icalparameter_set_altrep(icalparameter* value, const char* v);

/* CN */
icalparameter* icalparameter_new_cn(const char* v);
const char* icalparameter_get_cn(icalparameter* value);
void icalparameter_set_cn(icalparameter* value, const char* v);

/* CUTYPE */
icalparameter* icalparameter_new_cutype(icalparameter_cutype v);
icalparameter_cutype icalparameter_get_cutype(icalparameter* value);
void icalparameter_set_cutype(icalparameter* value, icalparameter_cutype v);

/* DELEGATED-FROM */
icalparameter* icalparameter_new_delegatedfrom(const char* v);
const char* icalparameter_get_delegatedfrom(icalparameter* value);
void icalparameter_set_delegatedfrom(icalparameter* value, const char* v);

/* DELEGATED-TO */
icalparameter* icalparameter_new_delegatedto(const char* v);
const char* icalparameter_get_delegatedto(icalparameter* value);
void icalparameter_set_delegatedto(icalparameter* value, const char* v);

/* DIR */
icalparameter* icalparameter_new_dir(const char* v);
const char* icalparameter_get_dir(icalparameter* value);
void icalparameter_set_dir(icalparameter* value, const char* v);

/* ENCODING */
icalparameter* icalparameter_new_encoding(icalparameter_encoding v);
icalparameter_encoding icalparameter_get_encoding(icalparameter* value);
void icalparameter_set_encoding(icalparameter* value, icalparameter_encoding v);

/* FBTYPE */
icalparameter* icalparameter_new_fbtype(icalparameter_fbtype v);
icalparameter_fbtype icalparameter_get_fbtype(icalparameter* value);
void icalparameter_set_fbtype(icalparameter* value, icalparameter_fbtype v);

/* FMTTYPE */
icalparameter* icalparameter_new_fmttype(const char* v);
const char* icalparameter_get_fmttype(icalparameter* value);
void icalparameter_set_fmttype(icalparameter* value, const char* v);

/* LANGUAGE */
icalparameter* icalparameter_new_language(const char* v);
const char* icalparameter_get_language(icalparameter* value);
void icalparameter_set_language(icalparameter* value, const char* v);

/* MEMBER */
icalparameter* icalparameter_new_member(const char* v);
const char* icalparameter_get_member(icalparameter* value);
void icalparameter_set_member(icalparameter* value, const char* v);

/* PARTSTAT */
icalparameter* icalparameter_new_partstat(icalparameter_partstat v);
icalparameter_partstat icalparameter_get_partstat(icalparameter* value);
void icalparameter_set_partstat(icalparameter* value, icalparameter_partstat v);

/* RANGE */
icalparameter* icalparameter_new_range(icalparameter_range v);
icalparameter_range icalparameter_get_range(icalparameter* value);
void icalparameter_set_range(icalparameter* value, icalparameter_range v);

/* RELATED */
icalparameter* icalparameter_new_related(icalparameter_related v);
icalparameter_related icalparameter_get_related(icalparameter* value);
void icalparameter_set_related(icalparameter* value, icalparameter_related v);

/* RELTYPE */
icalparameter* icalparameter_new_reltype(icalparameter_reltype v);
icalparameter_reltype icalparameter_get_reltype(icalparameter* value);
void icalparameter_set_reltype(icalparameter* value, icalparameter_reltype v);

/* ROLE */
icalparameter* icalparameter_new_role(icalparameter_role v);
icalparameter_role icalparameter_get_role(icalparameter* value);
void icalparameter_set_role(icalparameter* value, icalparameter_role v);

/* RSVP */
icalparameter* icalparameter_new_rsvp(int v);
int icalparameter_get_rsvp(icalparameter* value);
void icalparameter_set_rsvp(icalparameter* value, int v);

/* SENT-BY */
icalparameter* icalparameter_new_sentby(const char* v);
const char* icalparameter_get_sentby(icalparameter* value);
void icalparameter_set_sentby(icalparameter* value, const char* v);

/* TZID */
icalparameter* icalparameter_new_tzid(const char* v);
const char* icalparameter_get_tzid(icalparameter* value);
void icalparameter_set_tzid(icalparameter* value, const char* v);

/* VALUE */
icalparameter* icalparameter_new_value(icalparameter_value v);
icalparameter_value icalparameter_get_value(icalparameter* value);
void icalparameter_set_value(icalparameter* value, icalparameter_value v);

/* X */
icalparameter* icalparameter_new_x(const char* v);
const char* icalparameter_get_x(icalparameter* value);
void icalparameter_set_x(icalparameter* value, const char* v);

/* X-LIC-ERRORTYPE */
icalparameter* icalparameter_new_xlicerrortype(icalparameter_xlicerrortype v);
icalparameter_xlicerrortype icalparameter_get_xlicerrortype(icalparameter* value);
void icalparameter_set_xlicerrortype(icalparameter* value, icalparameter_xlicerrortype v);

/* X-LIC-COMPARETYPE */
icalparameter* icalparameter_new_xliccomparetype(icalparameter_xliccomparetype v);
icalparameter_xliccomparetype icalparameter_get_xliccomparetype(icalparameter* value);
void icalparameter_set_xliccomparetype(icalparameter* value, icalparameter_xliccomparetype v);

#endif /*ICALPARAMETER_H*/
/* -*- Mode: C -*-
  ======================================================================
  FILE: icalderivedproperties.{c,h}
  CREATOR: eric 09 May 1999
  
  $Id$
    
 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
 ======================================================================*/


#ifndef ICALPROPERTY_H
#define ICALPROPERTY_H

#include <time.h>

typedef void icalproperty;

icalproperty* icalproperty_new(icalproperty_kind kind);

icalproperty* icalproperty_new_clone(icalproperty * prop);

icalproperty* icalproperty_new_from_string(char* str);

char* icalproperty_as_ical_string(icalproperty* prop);

void  icalproperty_free(icalproperty* prop);

icalproperty_kind icalproperty_isa(icalproperty* property);
int icalproperty_isa_property(void* property);

void icalproperty_add_parameter(icalproperty* prop,icalparameter* parameter);
void icalproperty_set_parameter(icalproperty* prop,icalparameter* parameter);

void icalproperty_remove_parameter(icalproperty* prop,
				   icalparameter_kind kind);

int icalproperty_count_parameters(icalproperty* prop);

/* Iterate through the parameters */
icalparameter* icalproperty_get_first_parameter(icalproperty* prop,
						icalparameter_kind kind);
icalparameter* icalproperty_get_next_parameter(icalproperty* prop,
						icalparameter_kind kind);
/* Access the value of the property */
void icalproperty_set_value(icalproperty* prop, icalvalue* value);
icalvalue* icalproperty_get_value(icalproperty* prop);

/* Deal with X properties */

void icalproperty_set_x_name(icalproperty* prop, char* name);
char* icalproperty_get_x_name(icalproperty* prop);

/* Everything below this line is machine generated. Do not edit. */

/* METHOD */
icalproperty* icalproperty_new_method(icalproperty_method v);
icalproperty* icalproperty_vanew_method(icalproperty_method v, ...);
void icalproperty_set_method(icalproperty* prop, icalproperty_method v);
icalproperty_method icalproperty_get_method(icalproperty* prop);

/* X-LIC-MIMECID */
icalproperty* icalproperty_new_xlicmimecid(const char* v);
icalproperty* icalproperty_vanew_xlicmimecid(const char* v, ...);
void icalproperty_set_xlicmimecid(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimecid(icalproperty* prop);

/* LAST-MODIFIED */
icalproperty* icalproperty_new_lastmodified(struct icaltimetype v);
icalproperty* icalproperty_vanew_lastmodified(struct icaltimetype v, ...);
void icalproperty_set_lastmodified(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_lastmodified(icalproperty* prop);

/* UID */
icalproperty* icalproperty_new_uid(const char* v);
icalproperty* icalproperty_vanew_uid(const char* v, ...);
void icalproperty_set_uid(icalproperty* prop, const char* v);
const char* icalproperty_get_uid(icalproperty* prop);

/* PRODID */
icalproperty* icalproperty_new_prodid(const char* v);
icalproperty* icalproperty_vanew_prodid(const char* v, ...);
void icalproperty_set_prodid(icalproperty* prop, const char* v);
const char* icalproperty_get_prodid(icalproperty* prop);

/* STATUS */
icalproperty* icalproperty_new_status(icalproperty_status v);
icalproperty* icalproperty_vanew_status(icalproperty_status v, ...);
void icalproperty_set_status(icalproperty* prop, icalproperty_status v);
icalproperty_status icalproperty_get_status(icalproperty* prop);

/* DESCRIPTION */
icalproperty* icalproperty_new_description(const char* v);
icalproperty* icalproperty_vanew_description(const char* v, ...);
void icalproperty_set_description(icalproperty* prop, const char* v);
const char* icalproperty_get_description(icalproperty* prop);

/* DURATION */
icalproperty* icalproperty_new_duration(struct icaldurationtype v);
icalproperty* icalproperty_vanew_duration(struct icaldurationtype v, ...);
void icalproperty_set_duration(icalproperty* prop, struct icaldurationtype v);
struct icaldurationtype icalproperty_get_duration(icalproperty* prop);

/* CATEGORIES */
icalproperty* icalproperty_new_categories(const char* v);
icalproperty* icalproperty_vanew_categories(const char* v, ...);
void icalproperty_set_categories(icalproperty* prop, const char* v);
const char* icalproperty_get_categories(icalproperty* prop);

/* VERSION */
icalproperty* icalproperty_new_version(const char* v);
icalproperty* icalproperty_vanew_version(const char* v, ...);
void icalproperty_set_version(icalproperty* prop, const char* v);
const char* icalproperty_get_version(icalproperty* prop);

/* TZOFFSETFROM */
icalproperty* icalproperty_new_tzoffsetfrom(int v);
icalproperty* icalproperty_vanew_tzoffsetfrom(int v, ...);
void icalproperty_set_tzoffsetfrom(icalproperty* prop, int v);
int icalproperty_get_tzoffsetfrom(icalproperty* prop);

/* RRULE */
icalproperty* icalproperty_new_rrule(struct icalrecurrencetype v);
icalproperty* icalproperty_vanew_rrule(struct icalrecurrencetype v, ...);
void icalproperty_set_rrule(icalproperty* prop, struct icalrecurrencetype v);
struct icalrecurrencetype icalproperty_get_rrule(icalproperty* prop);

/* ATTENDEE */
icalproperty* icalproperty_new_attendee(const char* v);
icalproperty* icalproperty_vanew_attendee(const char* v, ...);
void icalproperty_set_attendee(icalproperty* prop, const char* v);
const char* icalproperty_get_attendee(icalproperty* prop);

/* CONTACT */
icalproperty* icalproperty_new_contact(const char* v);
icalproperty* icalproperty_vanew_contact(const char* v, ...);
void icalproperty_set_contact(icalproperty* prop, const char* v);
const char* icalproperty_get_contact(icalproperty* prop);

/* X-LIC-MIMECONTENTTYPE */
icalproperty* icalproperty_new_xlicmimecontenttype(const char* v);
icalproperty* icalproperty_vanew_xlicmimecontenttype(const char* v, ...);
void icalproperty_set_xlicmimecontenttype(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimecontenttype(icalproperty* prop);

/* X-LIC-MIMEOPTINFO */
icalproperty* icalproperty_new_xlicmimeoptinfo(const char* v);
icalproperty* icalproperty_vanew_xlicmimeoptinfo(const char* v, ...);
void icalproperty_set_xlicmimeoptinfo(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimeoptinfo(icalproperty* prop);

/* RELATED-TO */
icalproperty* icalproperty_new_relatedto(const char* v);
icalproperty* icalproperty_vanew_relatedto(const char* v, ...);
void icalproperty_set_relatedto(icalproperty* prop, const char* v);
const char* icalproperty_get_relatedto(icalproperty* prop);

/* ORGANIZER */
icalproperty* icalproperty_new_organizer(const char* v);
icalproperty* icalproperty_vanew_organizer(const char* v, ...);
void icalproperty_set_organizer(icalproperty* prop, const char* v);
const char* icalproperty_get_organizer(icalproperty* prop);

/* COMMENT */
icalproperty* icalproperty_new_comment(const char* v);
icalproperty* icalproperty_vanew_comment(const char* v, ...);
void icalproperty_set_comment(icalproperty* prop, const char* v);
const char* icalproperty_get_comment(icalproperty* prop);

/* X-LIC-ERROR */
icalproperty* icalproperty_new_xlicerror(const char* v);
icalproperty* icalproperty_vanew_xlicerror(const char* v, ...);
void icalproperty_set_xlicerror(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicerror(icalproperty* prop);

/* TRIGGER */
icalproperty* icalproperty_new_trigger(union icaltriggertype v);
icalproperty* icalproperty_vanew_trigger(union icaltriggertype v, ...);
void icalproperty_set_trigger(icalproperty* prop, union icaltriggertype v);
union icaltriggertype icalproperty_get_trigger(icalproperty* prop);

/* CLASS */
icalproperty* icalproperty_new_class(const char* v);
icalproperty* icalproperty_vanew_class(const char* v, ...);
void icalproperty_set_class(icalproperty* prop, const char* v);
const char* icalproperty_get_class(icalproperty* prop);

/* X */
icalproperty* icalproperty_new_x(const char* v);
icalproperty* icalproperty_vanew_x(const char* v, ...);
void icalproperty_set_x(icalproperty* prop, const char* v);
const char* icalproperty_get_x(icalproperty* prop);

/* TZOFFSETTO */
icalproperty* icalproperty_new_tzoffsetto(int v);
icalproperty* icalproperty_vanew_tzoffsetto(int v, ...);
void icalproperty_set_tzoffsetto(icalproperty* prop, int v);
int icalproperty_get_tzoffsetto(icalproperty* prop);

/* TRANSP */
icalproperty* icalproperty_new_transp(const char* v);
icalproperty* icalproperty_vanew_transp(const char* v, ...);
void icalproperty_set_transp(icalproperty* prop, const char* v);
const char* icalproperty_get_transp(icalproperty* prop);

/* X-LIC-MIMEENCODING */
icalproperty* icalproperty_new_xlicmimeencoding(const char* v);
icalproperty* icalproperty_vanew_xlicmimeencoding(const char* v, ...);
void icalproperty_set_xlicmimeencoding(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimeencoding(icalproperty* prop);

/* SEQUENCE */
icalproperty* icalproperty_new_sequence(int v);
icalproperty* icalproperty_vanew_sequence(int v, ...);
void icalproperty_set_sequence(icalproperty* prop, int v);
int icalproperty_get_sequence(icalproperty* prop);

/* LOCATION */
icalproperty* icalproperty_new_location(const char* v);
icalproperty* icalproperty_vanew_location(const char* v, ...);
void icalproperty_set_location(icalproperty* prop, const char* v);
const char* icalproperty_get_location(icalproperty* prop);

/* REQUEST-STATUS */
icalproperty* icalproperty_new_requeststatus(const char* v);
icalproperty* icalproperty_vanew_requeststatus(const char* v, ...);
void icalproperty_set_requeststatus(icalproperty* prop, const char* v);
const char* icalproperty_get_requeststatus(icalproperty* prop);

/* EXDATE */
icalproperty* icalproperty_new_exdate(struct icaltimetype v);
icalproperty* icalproperty_vanew_exdate(struct icaltimetype v, ...);
void icalproperty_set_exdate(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_exdate(icalproperty* prop);

/* TZID */
icalproperty* icalproperty_new_tzid(const char* v);
icalproperty* icalproperty_vanew_tzid(const char* v, ...);
void icalproperty_set_tzid(icalproperty* prop, const char* v);
const char* icalproperty_get_tzid(icalproperty* prop);

/* RESOURCES */
icalproperty* icalproperty_new_resources(const char* v);
icalproperty* icalproperty_vanew_resources(const char* v, ...);
void icalproperty_set_resources(icalproperty* prop, const char* v);
const char* icalproperty_get_resources(icalproperty* prop);

/* TZURL */
icalproperty* icalproperty_new_tzurl(const char* v);
icalproperty* icalproperty_vanew_tzurl(const char* v, ...);
void icalproperty_set_tzurl(icalproperty* prop, const char* v);
const char* icalproperty_get_tzurl(icalproperty* prop);

/* REPEAT */
icalproperty* icalproperty_new_repeat(int v);
icalproperty* icalproperty_vanew_repeat(int v, ...);
void icalproperty_set_repeat(icalproperty* prop, int v);
int icalproperty_get_repeat(icalproperty* prop);

/* PRIORITY */
icalproperty* icalproperty_new_priority(int v);
icalproperty* icalproperty_vanew_priority(int v, ...);
void icalproperty_set_priority(icalproperty* prop, int v);
int icalproperty_get_priority(icalproperty* prop);

/* FREEBUSY */
icalproperty* icalproperty_new_freebusy(struct icalperiodtype v);
icalproperty* icalproperty_vanew_freebusy(struct icalperiodtype v, ...);
void icalproperty_set_freebusy(icalproperty* prop, struct icalperiodtype v);
struct icalperiodtype icalproperty_get_freebusy(icalproperty* prop);

/* DTSTART */
icalproperty* icalproperty_new_dtstart(struct icaltimetype v);
icalproperty* icalproperty_vanew_dtstart(struct icaltimetype v, ...);
void icalproperty_set_dtstart(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_dtstart(icalproperty* prop);

/* RECURRENCE-ID */
icalproperty* icalproperty_new_recurrenceid(struct icaltimetype v);
icalproperty* icalproperty_vanew_recurrenceid(struct icaltimetype v, ...);
void icalproperty_set_recurrenceid(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_recurrenceid(icalproperty* prop);

/* SUMMARY */
icalproperty* icalproperty_new_summary(const char* v);
icalproperty* icalproperty_vanew_summary(const char* v, ...);
void icalproperty_set_summary(icalproperty* prop, const char* v);
const char* icalproperty_get_summary(icalproperty* prop);

/* DTEND */
icalproperty* icalproperty_new_dtend(struct icaltimetype v);
icalproperty* icalproperty_vanew_dtend(struct icaltimetype v, ...);
void icalproperty_set_dtend(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_dtend(icalproperty* prop);

/* TZNAME */
icalproperty* icalproperty_new_tzname(const char* v);
icalproperty* icalproperty_vanew_tzname(const char* v, ...);
void icalproperty_set_tzname(icalproperty* prop, const char* v);
const char* icalproperty_get_tzname(icalproperty* prop);

/* RDATE */
icalproperty* icalproperty_new_rdate(struct icalperiodtype v);
icalproperty* icalproperty_vanew_rdate(struct icalperiodtype v, ...);
void icalproperty_set_rdate(icalproperty* prop, struct icalperiodtype v);
struct icalperiodtype icalproperty_get_rdate(icalproperty* prop);

/* X-LIC-MIMEFILENAME */
icalproperty* icalproperty_new_xlicmimefilename(const char* v);
icalproperty* icalproperty_vanew_xlicmimefilename(const char* v, ...);
void icalproperty_set_xlicmimefilename(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimefilename(icalproperty* prop);

/* URL */
icalproperty* icalproperty_new_url(const char* v);
icalproperty* icalproperty_vanew_url(const char* v, ...);
void icalproperty_set_url(icalproperty* prop, const char* v);
const char* icalproperty_get_url(icalproperty* prop);

/* X-LIC-CLUSTERCOUNT */
icalproperty* icalproperty_new_xlicclustercount(int v);
icalproperty* icalproperty_vanew_xlicclustercount(int v, ...);
void icalproperty_set_xlicclustercount(icalproperty* prop, int v);
int icalproperty_get_xlicclustercount(icalproperty* prop);

/* ATTACH */
icalproperty* icalproperty_new_attach(struct icalattachtype v);
icalproperty* icalproperty_vanew_attach(struct icalattachtype v, ...);
void icalproperty_set_attach(icalproperty* prop, struct icalattachtype v);
struct icalattachtype icalproperty_get_attach(icalproperty* prop);

/* EXRULE */
icalproperty* icalproperty_new_exrule(struct icalrecurrencetype v);
icalproperty* icalproperty_vanew_exrule(struct icalrecurrencetype v, ...);
void icalproperty_set_exrule(icalproperty* prop, struct icalrecurrencetype v);
struct icalrecurrencetype icalproperty_get_exrule(icalproperty* prop);

/* QUERY */
icalproperty* icalproperty_new_query(const char* v);
icalproperty* icalproperty_vanew_query(const char* v, ...);
void icalproperty_set_query(icalproperty* prop, const char* v);
const char* icalproperty_get_query(icalproperty* prop);

/* PERCENT-COMPLETE */
icalproperty* icalproperty_new_percentcomplete(int v);
icalproperty* icalproperty_vanew_percentcomplete(int v, ...);
void icalproperty_set_percentcomplete(icalproperty* prop, int v);
int icalproperty_get_percentcomplete(icalproperty* prop);

/* CALSCALE */
icalproperty* icalproperty_new_calscale(const char* v);
icalproperty* icalproperty_vanew_calscale(const char* v, ...);
void icalproperty_set_calscale(icalproperty* prop, const char* v);
const char* icalproperty_get_calscale(icalproperty* prop);

/* CREATED */
icalproperty* icalproperty_new_created(struct icaltimetype v);
icalproperty* icalproperty_vanew_created(struct icaltimetype v, ...);
void icalproperty_set_created(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_created(icalproperty* prop);

/* GEO */
icalproperty* icalproperty_new_geo(struct icalgeotype v);
icalproperty* icalproperty_vanew_geo(struct icalgeotype v, ...);
void icalproperty_set_geo(icalproperty* prop, struct icalgeotype v);
struct icalgeotype icalproperty_get_geo(icalproperty* prop);

/* X-LIC-MIMECHARSET */
icalproperty* icalproperty_new_xlicmimecharset(const char* v);
icalproperty* icalproperty_vanew_xlicmimecharset(const char* v, ...);
void icalproperty_set_xlicmimecharset(icalproperty* prop, const char* v);
const char* icalproperty_get_xlicmimecharset(icalproperty* prop);

/* COMPLETED */
icalproperty* icalproperty_new_completed(struct icaltimetype v);
icalproperty* icalproperty_vanew_completed(struct icaltimetype v, ...);
void icalproperty_set_completed(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_completed(icalproperty* prop);

/* DTSTAMP */
icalproperty* icalproperty_new_dtstamp(struct icaltimetype v);
icalproperty* icalproperty_vanew_dtstamp(struct icaltimetype v, ...);
void icalproperty_set_dtstamp(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_dtstamp(icalproperty* prop);

/* DUE */
icalproperty* icalproperty_new_due(struct icaltimetype v);
icalproperty* icalproperty_vanew_due(struct icaltimetype v, ...);
void icalproperty_set_due(icalproperty* prop, struct icaltimetype v);
struct icaltimetype icalproperty_get_due(icalproperty* prop);

/* ACTION */
icalproperty* icalproperty_new_action(const char* v);
icalproperty* icalproperty_vanew_action(const char* v, ...);
void icalproperty_set_action(icalproperty* prop, const char* v);
const char* icalproperty_get_action(icalproperty* prop);
#endif /*ICALPROPERTY_H*/
/*======================================================================
 FILE: pvl.h
 CREATOR: eric November, 1995


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
======================================================================*/


#ifndef __PVL_H__
#define __PVL_H__

typedef void* pvl_list;
typedef void* pvl_elem;

/*
  struct pvl_elem_t

  This type is private. Always use pvl_elem instead. The struct would
  not even appear in this header except to make code in the USE_MACROS
  blocks work

  */
typedef struct pvl_elem_t
{
	int MAGIC;			/* Magic Identifier */
	void *d;			/* Pointer to data user is storing */
	struct pvl_elem_t *next;	/* Next element */
	struct pvl_elem_t *prior;	/* prior element */
} pvl_elem_t;



/* This global is incremented for each call to pvl_new_element(); it gives each
 * list a unique identifer */

extern int  pvl_elem_count;
extern int  pvl_list_count;

/* Create new lists or elements */
pvl_elem pvl_new_element(void* d, pvl_elem next,pvl_elem prior);
pvl_list pvl_newlist(void);
void pvl_free(pvl_list);

/* Add, remove, or get the head of the list */
void pvl_unshift(pvl_list l,void *d);
void* pvl_shift(pvl_list l);
pvl_elem pvl_head(pvl_list);

/* Add, remove or get the tail of the list */
void pvl_push(pvl_list l,void *d);
void* pvl_pop(pvl_list l);
pvl_elem pvl_tail(pvl_list);

/* Insert elements in random places */
typedef int (*pvl_comparef)(void* a, void* b); /* a, b are of the data type*/
void pvl_insert_ordered(pvl_list l,pvl_comparef f,void *d);
void pvl_insert_after(pvl_list l,pvl_elem e,void *d);
void pvl_insert_before(pvl_list l,pvl_elem e,void *d);

/* Remove an element, or clear the entire list */
void* pvl_remove(pvl_list,pvl_elem); /* Remove element, return data */
void pvl_clear(pvl_list); /* Remove all elements, de-allocate all data */

int pvl_count(pvl_list);

/* Navagate the list */
pvl_elem pvl_next(pvl_elem e);
pvl_elem pvl_prior(pvl_elem e);

/* get the data in the list */
#ifndef PVL_USE_MACROS
void* pvl_data(pvl_elem);
#else
#define pvl_data(x) x==0 ? 0 : ((struct pvl_elem_t *)x)->d;
#endif


/* Find an element for which a function returns true */
typedef int (*pvl_findf)(void* a, void* b); /*a is list elem, b is other data*/
pvl_elem pvl_find(pvl_list l,pvl_findf f,void* v);
pvl_elem pvl_find_next(pvl_list l,pvl_findf f,void* v);

/* Pass each element in the list to a function */
typedef void (*pvl_applyf)(void* a, void* b); /*a is list elem, b is other data*/
void pvl_apply(pvl_list l,pvl_applyf f, void *v);


#endif /* __PVL_H__ */





/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalcomponent.h
 CREATOR: eric 20 March 1999


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalcomponent.h

======================================================================*/

#ifndef ICALCOMPONENT_H
#define ICALCOMPONENT_H


typedef void icalcomponent;

/* This is exposed so that callers will not have to allocate and
   deallocate iterators. Pretend that you can't see it. */
typedef struct icalcompiter
{
	icalcomponent_kind kind;
	pvl_elem iter;

} icalcompiter;

icalcomponent* icalcomponent_new(icalcomponent_kind kind);
icalcomponent* icalcomponent_new_clone(icalcomponent* component);
icalcomponent* icalcomponent_new_from_string(char* str);
icalcomponent* icalcomponent_vanew(icalcomponent_kind kind, ...);
void icalcomponent_free(icalcomponent* component);

char* icalcomponent_as_ical_string(icalcomponent* component);

int icalcomponent_is_valid(icalcomponent* component);

icalcomponent_kind icalcomponent_isa(icalcomponent* component);

int icalcomponent_isa_component (void* component);

/* 
 * Working with properties
 */

void icalcomponent_add_property(icalcomponent* component,
				icalproperty* property);

void icalcomponent_remove_property(icalcomponent* component,
				   icalproperty* property);

int icalcomponent_count_properties(icalcomponent* component,
				   icalproperty_kind kind);

/* Iterate through the properties */
icalproperty* icalcomponent_get_current_property(icalcomponent* component);

icalproperty* icalcomponent_get_first_property(icalcomponent* component,
					      icalproperty_kind kind);
icalproperty* icalcomponent_get_next_property(icalcomponent* component,
					      icalproperty_kind kind);

/* Return a null-terminated array of icalproperties*/

icalproperty** icalcomponent_get_properties(icalcomponent* component,
					      icalproperty_kind kind);


/* 
 * Working with components
 */ 


void icalcomponent_add_component(icalcomponent* parent,
				icalcomponent* child);

void icalcomponent_remove_component(icalcomponent* parent,
				icalcomponent* child);

int icalcomponent_count_components(icalcomponent* component,
				   icalcomponent_kind kind);

/* Iteration Routines. There are two forms of iterators, internal and
external. The internal ones came first, and are almost completely
sufficient, but they fail badly when you want to construct a loop that
removes components from the container.

The internal iterators are deprecated. */

/* Using external iterators */
icalcompiter icalcomponent_begin_component(icalcomponent* component,
					   icalcomponent_kind kind);

icalcompiter icalcomponent_end_component(icalcomponent* component,
					 icalcomponent_kind kind);

/* Iterate through components */
icalcomponent* icalcomponent_get_current_component (icalcomponent* component);

icalcomponent* icalcomponent_get_first_component(icalcomponent* component,
					      icalcomponent_kind kind);
icalcomponent* icalcomponent_get_next_component(icalcomponent* component,
					      icalcomponent_kind kind);




/* Working with embedded error properties */

int icalcomponent_count_errors(icalcomponent* component);

/* Remove all X-LIC-ERROR properties*/
void icalcomponent_strip_errors(icalcomponent* component);

/* Convert some X-LIC-ERROR properties into RETURN-STATUS properties*/
void icalcomponent_convert_errors(icalcomponent* component);

/* Internal operations. You don't see these... */
icalcomponent* icalcomponent_get_parent(icalcomponent* component);
void icalcomponent_set_parent(icalcomponent* component, 
			      icalcomponent* parent);
/* External component iterator */
icalcomponent* icalcompiter_next(icalcompiter* i);
icalcomponent* icalcompiter_prior(icalcompiter* i);
icalcomponent* icalcompiter_deref(icalcompiter* i);

/************* Derived class methods.  ****************************

If the code was in an OO language, the remaining routines would be
members of classes derived from icalcomponent. Don't call them on the
wrong component subtypes. */

/* For VCOMPONENT: Return a reference to the first VEVENT, VTODO or
   VJOURNAL */
icalcomponent* icalcomponent_get_first_real_component(icalcomponent *c);

/* For VEVENT, VTODO, VJOURNAL and VTIMEZONE: report the start and end
   times of an event in UTC */
struct icaltime_span icalcomponent_get_span(icalcomponent* comp);

/******************** Convienience routines **********************/

void icalcomponent_set_dtstart(icalcomponent* comp, struct icaltimetype v);
struct icaltimetype icalcomponent_get_dtstart(icalcomponent* comp);

/* For the icalcomponent routines only, dtend and duration are tied
   together. If you call the set routine for one and the other exists,
   the routine will calculate the change to the other. That is, if
   there is a DTEND and you call set_duration, the routine will modify
   DTEND to be the sum of DTSTART and the duration. If you call a get
   routine for one and the other exists, the routine will calculate
   the return value. If you call a set routine and neither exists, the
   routine will create the apcompriate comperty */


struct icaltimetype icalcomponent_get_dtend(icalcomponent* comp);
void icalcomponent_set_dtend(icalcomponent* comp, struct icaltimetype v);

void icalcomponent_set_duration(icalcomponent* comp, 
				struct icaldurationtype v);
struct icaldurationtype icalcomponent_get_duration(icalcomponent* comp);

void icalcomponent_set_method(icalcomponent* comp, icalproperty_method method);
icalproperty_method icalcomponent_get_method(icalcomponent* comp);

struct icaltimetype icalcomponent_get_dtstamp(icalcomponent* comp);
void icalcomponent_set_dtstamp(icalcomponent* comp, struct icaltimetype v);


void icalcomponent_set_summary(icalcomponent* comp, const char* v);
const char* icalcomponent_get_summary(icalcomponent* comp);

void icalcomponent_set_comment(icalcomponent* comp, const char* v);
const char* icalcomponent_get_comment(icalcomponent* comp);

void icalcomponent_set_organizer(icalcomponent* comp, const char* v);
const char* icalcomponent_get_organizer(icalcomponent* comp);

void icalcomponent_set_uid(icalcomponent* comp, const char* v);
const char* icalcomponent_get_uid(icalcomponent* comp);

void icalcomponent_set_recurrenceid(icalcomponent* comp, 
				    struct icaltimetype v);
struct icaltimetype icalcomponent_get_recurrenceid(icalcomponent* comp);

/*************** Type Specific routines ***************/

icalcomponent* icalcomponent_new_vcalendar();
icalcomponent* icalcomponent_new_vevent();
icalcomponent* icalcomponent_new_vtodo();
icalcomponent* icalcomponent_new_vjournal();
icalcomponent* icalcomponent_new_vfreebusy();
icalcomponent* icalcomponent_new_vtimezone();
icalcomponent* icalcomponent_new_xstandard();
icalcomponent* icalcomponent_new_xdaylight();



#endif /* !ICALCOMPONENT_H */



/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalparser.h
  CREATOR: eric 20 April 1999
  
  $Id$


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalparser.h

======================================================================*/


#ifndef ICALPARSER_H
#define ICALPARSER_H


#include <stdio.h> /* For FILE* */

typedef void* icalparser;


/***********************************************************************
 * Line-oriented parsing. 
 * 
 * Create a new parser via icalparse_new_parser, then add ines one at
 * a time with icalparse_add_line(). icalparser_add_line() will return
 * non-zero when it has finished with a component.
 ***********************************************************************/

typedef enum icalparser_state {
    ICALPARSER_ERROR,
    ICALPARSER_SUCCESS,
    ICALPARSER_BEGIN_COMP,
    ICALPARSER_END_COMP,
    ICALPARSER_IN_PROGRESS
} icalparser_state;

icalparser* icalparser_new(void);
icalcomponent* icalparser_add_line(icalparser* parser, char* str );
icalcomponent* icalparser_claim(icalparser* parser);
icalcomponent* icalparser_clean(icalparser* parser);
icalparser_state icalparser_get_state(icalparser* parser);
void icalparser_free(icalparser* parser);


/***********************************************************************
 * Message oriented parsing.  icalparser_parse takes a string that
 * holds the text ( in RFC 2445 format ) and returns a pointer to an
 * icalcomponent. The caller owns the memory. line_gen_func is a
 * pointer to a function that returns one content line per invocation
 **********************************************************************/

icalcomponent* icalparser_parse(icalparser *parser,
				char* (*line_gen_func)(char *s, size_t size, void *d));

/* Set the data that icalparser_parse will give to the line_gen_func
   as the parameter 'd'*/
void icalparser_set_gen_data(icalparser* parser, void* data);


icalcomponent* icalparser_parse_string(char* str);


/***********************************************************************
 * Parser support functions
 ***********************************************************************/

/* Use the flex/bison parser to turn a string into a value type */
icalvalue*  icalparser_parse_value(icalvalue_kind kind, 
				   const char* str, icalcomponent** errors);

/* Given a line generator function, return a single iCal content line.*/
char* icalparser_get_line(icalparser* parser, char* (*line_gen_func)(char *s, size_t size, void *d));

char* string_line_generator(char *out, size_t buf_size, void *d);

#endif /* !ICALPARSE_H */
/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalmemory.h
 CREATOR: eric 30 June 1999


 $Id$
 $Locker:  $

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

 The Initial Developer of the Original Code is Eric Busboom

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
======================================================================*/

#ifndef ICALMEMORY_H
#define ICALMEMORY_H

#include <sys/types.h> /* for size_t */


/* Tmp buffers are managed by ical. References can be returned to the
   caller, although the caller will not own the memory. */

void* icalmemory_tmp_buffer(size_t size);
char* icalmemory_tmp_copy(const char* str);

/* Add an externally allocated buffer to the ring. */
void  icalmemory_add_tmp_buffer(void*);


/* Free all memory used in the ring */
void icalmemory_free_ring(void);

/* Non-tmp buffers must be freed. These are mostly wrappers around
 * malloc, etc, but are used so the caller can change the memory
 * allocators in a future version of the library */

void* icalmemory_new_buffer(size_t size);
void* icalmemory_resize_buffer(void* buf, size_t size);
void icalmemory_free_buffer(void* buf);

/* icalmemory_append_string will copy the string 'string' to the
   buffer 'buf' starting at position 'pos', reallocing 'buf' if it is
   too small. 'buf_size' is the size of 'buf' and will be changed if
   'buf' is reallocated. 'pos' will point to the last byte of the new
   string in 'buf', usually a '\0' */

/* THESE ROUTINES CAN NOT BE USED ON TMP BUFFERS. Only use them on
   normally allocated memory, or on buffers created from
   icalmemory_new_buffer, never with buffers created by
   icalmemory_tmp_buffer. If icalmemory_append_string has to resize a
   buffer on the ring, the ring will loose track of it an you will
   have memory problems. */

void icalmemory_append_string(char** buf, char** pos, size_t* buf_size, 
			      const char* string);

/*  icalmemory_append_char is similar, but is appends a character instead of a string */
void icalmemory_append_char(char** buf, char** pos, size_t* buf_size, 
			      char ch);

/* A wrapper around strdup */
char* icalmemory_strdup(const char *s);

#endif /* !ICALMEMORY_H */



/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalerror.h
  CREATOR: eric 09 May 1999
  
  $Id$


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalerror.h

======================================================================*/


#ifndef ICALERROR_H
#define ICALERROR_H

#include <assert.h>
#include <stdio.h> /* For icalerror_warn() */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* This routine is called before any error is triggered. It is called
   by icalerror_set_errno, so it does not appear in all of the macros
   below */
void icalerror_stop_here(void);

void icalerror_crash_here(void);

#ifdef ICAL_ERRORS_ARE_FATAL
#undef NDEBUG
#endif

#define icalerror_check_value_type(value,type);
#define icalerror_check_property_type(value,type);
#define icalerror_check_parameter_type(value,type);
#define icalerror_check_component_type(value,type);

/* Assert with a message */
#ifdef ICAL_ERRORS_ARE_FATAL

#ifdef __GNUC__
#define icalerror_assert(test,message) if(!(test)){fprintf(stderr,"%s(), %s:%d: %s\n",__FUNCTION__,__FILE__,__LINE__,message);icalerror_stop_here(); abort();}
#else /*__GNUC__*/
#define icalerror_assert(test,message) if(!(test)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,message);icalerror_stop_here(); abort();}
#endif /*__GNUC__*/
#else
#define icalerror_assert(test,message) 
#endif 

/* Check & abort if check fails */
#ifdef ICAL_ERRORS_ARE_FATAL
#define icalerror_check_arg(test,arg) icalerror_stop_here();assert(test) 
#else
#define icalerror_check_arg(test,arg)
#endif
/* Check & return void if check fails*/

#ifdef ICAL_ERRORS_ARE_FATAL
#define icalerror_check_arg_rv(test,arg) icalerror_stop_here();assert(test);
#else 
#define icalerror_check_arg_rv(test,arg) if(!(test)) { icalerror_set_errno(ICAL_BADARG_ERROR); return; }
#endif

/* Check & return 0 if check fails*/
#ifdef ICAL_ERRORS_ARE_FATAL
#define icalerror_check_arg_rz(test,arg) icalerror_stop_here();assert(test); 
#else
#define icalerror_check_arg_rz(test,arg) if(!(test)) {icalerror_set_errno(ICAL_BADARG_ERROR); return 0;}
#endif


/* Check & return an error if check fails*/
#ifdef ICAL_ERRORS_ARE_FATAL
#define icalerror_check_arg_re(test,arg,error) icalerror_stop_here();assert(test); 
#else
#define icalerror_check_arg_re(test,arg,error) if(!(test)) {icalerror_stop_here(); return error;}
#endif


/* Warning messages */

#ifdef ICAL_ERRORS_ARE_FATAL 

#ifdef __GNUC__
#define icalerror_warn(message) {fprintf(stderr,"%s(), %s:%d: %s\n",__FUNCTION__,__FILE__,__LINE__,message); abort();}
#else /* __GNU_C__ */
#define icalerror_warn(message) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,message); abort();}
#endif /* __GNU_C__ */

#else /*ICAL_ERRORS_ARE_FATAL */

#ifdef __GNUC__
#define icalerror_warn(message) {fprintf(stderr,"%s(), %s:%d: %s\n",__FUNCTION__,__FILE__,__LINE__,message);}
#else /* __GNU_C__ */
#define icalerror_warn(message) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,message);}
#endif /* __GNU_C__ */

#endif /*ICAL_ERRORS_ARE_FATAL*/

typedef enum icalerrorenum {
    
    ICAL_BADARG_ERROR,
    ICAL_NEWFAILED_ERROR,
    ICAL_MALFORMEDDATA_ERROR, 
    ICAL_PARSE_ERROR,
    ICAL_INTERNAL_ERROR, /* Like assert --internal consist. prob */
    ICAL_FILE_ERROR,
    ICAL_ALLOCATION_ERROR,
    ICAL_USAGE_ERROR,
    ICAL_NO_ERROR,
    ICAL_MULTIPLEINCLUSION_ERROR,
    ICAL_TIMEDOUT_ERROR,
    ICAL_UNKNOWN_ERROR /* Used for problems in input to icalerror_strerror()*/

} icalerrorenum;

extern icalerrorenum icalerrno;


void icalerror_clear_errno(void);
void icalerror_set_errno(icalerrorenum);

char* icalerror_strerror(icalerrorenum e);


#endif /* !ICALERROR_H */



/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalrestriction.h
  CREATOR: eric 24 April 1999
  
  $Id$


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalrestriction.h

  Contributions from:
     Graham Davison (g.m.davison@computer.org)


======================================================================*/


#ifndef ICALRESTRICTION_H
#define ICALRESTRICTION_H

/* These must stay in this order for icalrestriction_compare to work */
typedef enum icalrestriction_kind {
    ICAL_RESTRICTION_NONE=0,		/* 0 */
    ICAL_RESTRICTION_ZERO,		/* 1 */
    ICAL_RESTRICTION_ONE,		/* 2 */
    ICAL_RESTRICTION_ZEROPLUS,		/* 3 */
    ICAL_RESTRICTION_ONEPLUS,		/* 4 */
    ICAL_RESTRICTION_ZEROORONE,		/* 5 */
    ICAL_RESTRICTION_ONEEXCLUSIVE,	/* 6 */
    ICAL_RESTRICTION_ONEMUTUAL,		/* 7 */
    ICAL_RESTRICTION_UNKNOWN		/* 8 */
} icalrestriction_kind;

int 
icalrestriction_compare(icalrestriction_kind restr, int count);


int
icalrestriction_is_parameter_allowed(icalproperty_kind property,
                                       icalparameter_kind parameter);

int icalrestriction_check(icalcomponent* comp);


#endif /* !ICALRESTRICTION_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalrecur.h
 CREATOR: eric 20 March 2000


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icaltypes.h

======================================================================*/

#ifndef ICALRECUR_H
#define ICALRECUR_H

#include <time.h>


typedef void icalrecur_iterator;
void icalrecurrencetype_test();


icalrecur_iterator* icalrecur_iterator_new(struct icalrecurrencetype rule, struct icaltimetype dtstart);

struct icaltimetype icalrecur_iterator_next(icalrecur_iterator*);

int icalrecur_iterator_count(icalrecur_iterator*);

void icalrecur_iterator_free(icalrecur_iterator*);


#endif
/* -*- Mode: C -*-
  ======================================================================
  FILE: sspm.h Mime Parser
  CREATOR: eric 25 June 2000
  
  $Id$
  $Locker:  $
    
 The contents of this file are subject to the Mozilla Public License
 Version 1.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 the License for the specific language governing rights and
 limitations under the License.
 

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The Initial Developer of the Original Code is Eric Busboom

 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org
 ======================================================================*/

#ifndef SSPM_H
#define SSPM_H

enum sspm_major_type {
    SSPM_NO_MAJOR_TYPE,
    SSPM_TEXT_MAJOR_TYPE,
    SSPM_IMAGE_MAJOR_TYPE,
    SSPM_AUDIO_MAJOR_TYPE,
    SSPM_VIDEO_MAJOR_TYPE,
    SSPM_APPLICATION_MAJOR_TYPE,
    SSPM_MULTIPART_MAJOR_TYPE,
    SSPM_MESSAGE_MAJOR_TYPE,
    SSPM_UNKNOWN_MAJOR_TYPE
};

enum sspm_minor_type {
    SSPM_NO_MINOR_TYPE,
    SSPM_ANY_MINOR_TYPE,
    SSPM_PLAIN_MINOR_TYPE,
    SSPM_RFC822_MINOR_TYPE,
    SSPM_DIGEST_MINOR_TYPE,
    SSPM_CALENDAR_MINOR_TYPE,
    SSPM_MIXED_MINOR_TYPE,
    SSPM_RELATED_MINOR_TYPE,
    SSPM_ALTERNATIVE_MINOR_TYPE,
    SSPM_PARALLEL_MINOR_TYPE,
    SSPM_UNKNOWN_MINOR_TYPE
};

enum sspm_encoding {
    SSPM_NO_ENCODING,
    SSPM_QUOTED_PRINTABLE_ENCODING,
    SSPM_8BIT_ENCODING,
    SSPM_7BIT_ENCODING,
    SSPM_BINARY_ENCODING,
    SSPM_BASE64_ENCODING,
    SSPM_UNKNOWN_ENCODING
};

enum sspm_error{
    SSPM_NO_ERROR,
    SSPM_UNEXPECTED_BOUNDARY_ERROR,
    SSPM_WRONG_BOUNDARY_ERROR,
    SSPM_NO_BOUNDARY_ERROR,
    SSPM_NO_HEADER_ERROR,
    SSPM_MALFORMED_HEADER_ERROR
};


struct sspm_header
{
	int def;
	char* boundary;
	enum sspm_major_type major;
	enum sspm_minor_type minor;
	char *minor_text;
	char ** content_type_params;
	char* charset;
	enum sspm_encoding encoding;
	char* filename;
	char* content_id;
	enum sspm_error error;
	char* error_text;
};

struct sspm_part {
	struct sspm_header header;
	int level;
	size_t data_size;
	void *data;
};

struct sspm_action_map {
	enum sspm_major_type major;
	enum sspm_minor_type minor;
	void* (*new_part)();
	void (*add_line)(void *part, struct sspm_header *header, 
			 char* line, size_t size);
	void* (*end_part)(void* part);
	void (*free_part)(void *part);
};

char* sspm_major_type_string(enum sspm_major_type type);
char* sspm_minor_type_string(enum sspm_major_type type);
char* sspm_encoding_string(enum sspm_encoding type);

int sspm_parse_mime(struct sspm_part *parts, 
		    size_t max_parts,
		    struct sspm_action_map *actions,
		    char* (*get_string)(char *s, size_t size, void* data),
		    void *get_string_data,
		    struct sspm_header *first_header
    );

void sspm_free_parts(struct sspm_part *parts, size_t max_parts);

char *decode_quoted_printable(char *dest, 
				       char *src,
				       size_t *size);
char *decode_base64(char *dest, 
			     char *src,
			     size_t *size);


int sspm_write_mime(struct sspm_part *parts,size_t num_parts,
		    char **output_string, char* header);

#endif /*SSPM_H*/
/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalmime.h
 CREATOR: eric 26 July 2000


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

======================================================================*/

#ifndef ICALMIME_H
#define ICALMIME_H


icalcomponent* icalmime_parse(	char* (*line_gen_func)(char *s, size_t size, 
						       void *d),
				void *data);

/* The inverse of icalmime_parse, not implemented yet. Use sspm.h directly.  */
char* icalmime_as_mime_string(char* component);



#endif /* !ICALMIME_H */



