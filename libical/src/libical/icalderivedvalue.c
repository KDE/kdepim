/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalvalue.c
  CREATOR: eric 02 May 1999
  
  $Id$


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalvalue.c

  Contributions from:
     Graham Davison (g.m.davison@computer.org)


======================================================================*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalerror.h"
#include "icalmemory.h"
#include "icalparser.h"
#include "icalenums.h"

#include "icalvalueimpl.h"

#include <stdlib.h> /* for malloc */
#include <stdio.h> /* for sprintf */
#include <string.h> /* For memset, others */
#include <stddef.h> /* For offsetof() macro */
#include <errno.h>
#include <time.h> /* for mktime */
#include <stdlib.h> /* for atoi and atof */
#include <limits.h> /* for SHRT_MAX */         

#if _MAC_OS_
#include "icalmemory_strdup.h"
#endif

#define TMP_BUF_SIZE 1024

struct icalvalue_impl*  icalvalue_new_impl(icalvalue_kind kind);

struct icalvalue_kind_map {
	icalvalue_kind kind;
	char name[20];
};

static struct icalvalue_kind_map value_map[] = 
{
    { ICAL_BINARY_VALUE, "BINARY"},
    { ICAL_BOOLEAN_VALUE, "BOOLEAN"},
    { ICAL_CALADDRESS_VALUE, "CAL-ADDRESS"},
    { ICAL_DATE_VALUE, "DATE"},
    { ICAL_DATETIME_VALUE, "DATE-TIME"},
    { ICAL_DURATION_VALUE, "DURATION"},
    { ICAL_FLOAT_VALUE, "FLOAT"},
    { ICAL_INTEGER_VALUE, "INTEGER"},
    { ICAL_PERIOD_VALUE, "PERIOD"},
    { ICAL_RECUR_VALUE, "RECUR"},
    { ICAL_TEXT_VALUE, "TEXT"},
    { ICAL_TIME_VALUE, "TIME"},
    { ICAL_URI_VALUE, "URI"},
    { ICAL_UTCOFFSET_VALUE, "UTC-OFFSET"},
    { ICAL_METHOD_VALUE, "METHOD"}, /* Not an RFC2445 type */
    { ICAL_STATUS_VALUE, "STATUS"}, /* Not an RFC2445 type */
    { ICAL_GEO_VALUE, "FLOAT"}, /* Not an RFC2445 type */
    { ICAL_ATTACH_VALUE, "ATTACH"}, /* Not an RFC2445 type */
    { ICAL_DATETIMEDATE_VALUE, "DATETIMEDATE"}, /* Not an RFC2445 type */
    { ICAL_DATETIMEPERIOD_VALUE, "DATETIMEPERIOD"}, /* Not an RFC2445 type */
    { ICAL_TRIGGER_VALUE, "TRIGGER"}, /* Not an RFC2445 type */
    { ICAL_QUERY_VALUE, "QUERY"},
    { ICAL_NO_VALUE, ""},
};

const char* icalvalue_kind_to_string(icalvalue_kind kind)
{
    int i;

    for (i=0; value_map[i].kind != ICAL_NO_VALUE; i++) {
	if (value_map[i].kind == kind) {
	    return value_map[i].name;
	}
    }

    return 0;
}

icalvalue_kind icalvalue_string_to_kind(const char* str)
{
    int i;

    for (i=0; value_map[i].kind != ICAL_NO_VALUE; i++) {
	if (strcmp(value_map[i].name,str) == 0) {
	    return value_map[i].kind;
	}
    }

    return  value_map[i].kind;

}



/* The remaining interfaces are 'new', 'set' and 'get' for each of the value
   types */


/* Everything below this line is machine generated. Do not edit. */



icalvalue* icalvalue_new_boolean (int v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_BOOLEAN_VALUE);
   
   icalvalue_set_boolean((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_boolean(icalvalue* value, int v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_BOOLEAN_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_int = v; 
 }
int icalvalue_get_boolean(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_BOOLEAN_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_int;
}



icalvalue* icalvalue_new_utcoffset (int v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_UTCOFFSET_VALUE);
   
   icalvalue_set_utcoffset((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_utcoffset(icalvalue* value, int v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_UTCOFFSET_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_int = v; 
 }
int icalvalue_get_utcoffset(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_UTCOFFSET_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_int;
}



icalvalue* icalvalue_new_method (enum icalproperty_method v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_METHOD_VALUE);
   
   icalvalue_set_method((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_method(icalvalue* value, enum icalproperty_method v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_METHOD_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_method = v; 
 }
enum icalproperty_method icalvalue_get_method(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_METHOD_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_method;
}



icalvalue* icalvalue_new_caladdress (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_CALADDRESS_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_caladdress((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_caladdress(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_CALADDRESS_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_caladdress(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_CALADDRESS_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_period (struct icalperiodtype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_PERIOD_VALUE);
   
   icalvalue_set_period((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_period(icalvalue* value, struct icalperiodtype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_PERIOD_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_period = v; 
 }
struct icalperiodtype icalvalue_get_period(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_PERIOD_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_period;
}



icalvalue* icalvalue_new_status (enum icalproperty_status v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_STATUS_VALUE);
   
   icalvalue_set_status((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_status(icalvalue* value, enum icalproperty_status v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_STATUS_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_status = v; 
 }
enum icalproperty_status icalvalue_get_status(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_STATUS_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_status;
}



icalvalue* icalvalue_new_binary (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_BINARY_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_binary((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_binary(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_BINARY_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_binary(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_BINARY_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_text (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_TEXT_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_text((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_text(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_TEXT_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_text(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_TEXT_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_datetimedate (struct icaltimetype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_DATETIMEDATE_VALUE);
   
   icalvalue_set_datetimedate((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_datetimedate(icalvalue* value, struct icaltimetype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_DATETIMEDATE_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_time = v; 
 }
struct icaltimetype icalvalue_get_datetimedate(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_DATETIMEDATE_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_time;
}



icalvalue* icalvalue_new_duration (struct icaldurationtype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_DURATION_VALUE);
   
   icalvalue_set_duration((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_duration(icalvalue* value, struct icaldurationtype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_DURATION_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_duration = v; 
 }
struct icaldurationtype icalvalue_get_duration(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_DURATION_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_duration;
}



icalvalue* icalvalue_new_integer (int v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_INTEGER_VALUE);
   
   icalvalue_set_integer((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_integer(icalvalue* value, int v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_INTEGER_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_int = v; 
 }
int icalvalue_get_integer(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_INTEGER_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_int;
}



icalvalue* icalvalue_new_time (struct icaltimetype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_TIME_VALUE);
   
   icalvalue_set_time((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_time(icalvalue* value, struct icaltimetype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_TIME_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_time = v; 
 }
struct icaltimetype icalvalue_get_time(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_TIME_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_time;
}



icalvalue* icalvalue_new_uri (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_URI_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_uri((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_uri(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_URI_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_uri(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_URI_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_attach (struct icalattachtype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_ATTACH_VALUE);
   
   icalvalue_set_attach((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_attach(icalvalue* value, struct icalattachtype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_ATTACH_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_attach = v; 
 }
struct icalattachtype icalvalue_get_attach(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_ATTACH_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_attach;
}



icalvalue* icalvalue_new_class (enum icalproperty_class v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_CLASS_VALUE);
   
   icalvalue_set_class((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_class(icalvalue* value, enum icalproperty_class v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_CLASS_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_class = v; 
 }
enum icalproperty_class icalvalue_get_class(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_CLASS_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_class;
}



icalvalue* icalvalue_new_float (float v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_FLOAT_VALUE);
   
   icalvalue_set_float((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_float(icalvalue* value, float v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_FLOAT_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_float = v; 
 }
float icalvalue_get_float(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_FLOAT_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_float;
}



icalvalue* icalvalue_new_query (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_QUERY_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_query((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_query(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_QUERY_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_query(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_QUERY_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_string (const char* v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_STRING_VALUE);
   icalerror_check_arg_rz( (v!=0),"v");

   icalvalue_set_string((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_string(icalvalue* value, const char* v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    icalerror_check_arg_rv( (v!=0),"v");

    icalerror_check_value_type(value, ICAL_STRING_VALUE);
    impl = (struct icalvalue_impl*)value;
    if(impl->data.v_string!=0) {free((void*)impl->data.v_string);}

    impl->data.v_string = strdup(v);

    if (impl->data.v_string == 0){
      errno = ENOMEM;
    }
 
 }
const char* icalvalue_get_string(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_STRING_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_string;
}



icalvalue* icalvalue_new_transp (enum icalproperty_transp v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_TRANSP_VALUE);
   
   icalvalue_set_transp((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_transp(icalvalue* value, enum icalproperty_transp v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_TRANSP_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_transp = v; 
 }
enum icalproperty_transp icalvalue_get_transp(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_TRANSP_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_transp;
}



icalvalue* icalvalue_new_datetime (struct icaltimetype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_DATETIME_VALUE);
   
   icalvalue_set_datetime((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_datetime(icalvalue* value, struct icaltimetype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_DATETIME_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_time = v; 
 }
struct icaltimetype icalvalue_get_datetime(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_DATETIME_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_time;
}



icalvalue* icalvalue_new_geo (struct icalgeotype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_GEO_VALUE);
   
   icalvalue_set_geo((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_geo(icalvalue* value, struct icalgeotype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_GEO_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_geo = v; 
 }
struct icalgeotype icalvalue_get_geo(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_GEO_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_geo;
}



icalvalue* icalvalue_new_date (struct icaltimetype v){
   struct icalvalue_impl* impl = icalvalue_new_impl(ICAL_DATE_VALUE);
   
   icalvalue_set_date((icalvalue*)impl,v);
   return (icalvalue*)impl;
}
void icalvalue_set_date(icalvalue* value, struct icaltimetype v) {
    struct icalvalue_impl* impl; 
    icalerror_check_arg_rv( (value!=0),"value");
    
    icalerror_check_value_type(value, ICAL_DATE_VALUE);
    impl = (struct icalvalue_impl*)value;

    impl->data.v_time = v; 
 }
struct icaltimetype icalvalue_get_date(icalvalue* value) {

    icalerror_check_arg( (value!=0),"value");
    icalerror_check_value_type(value, ICAL_DATE_VALUE);
    return ((struct icalvalue_impl*)value)->data.v_time;
}
