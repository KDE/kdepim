/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalset.h
 CREATOR: eric 28 November 1999


 Icalset is the "base class" for representations of a collection of
 iCal components. Derived classes (actually delegatees) include:
 
    icalfileset   Store componetns in a single file
    icaldirset    Store components in multiple files in a directory
    icalheapset   Store components on the heap
    icalmysqlset  Store components in a mysql database. 

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

#ifndef ICALSET_H
#define ICALSET_H


typedef void icalset;

typedef enum icalset_kind {
    ICAL_FILE_SET,
    ICAL_DIR_SET,
    ICAL_HEAP_SET,
    ICAL_MYSQL_SET,
    ICAL_CAP_SET
} icalset_kind;


/* Create a specific derived type of set */
icalset* icalset_new_file(const char* path);
icalset* icalset_new_dir(const char* path);
icalset* icalset_new_heap(void);
icalset* icalset_new_mysql(const char* path);
/*icalset* icalset_new_cap(icalcstp* cstp);*/

void icalset_free(icalset* set);

const char* icalset_path(icalset* set);

/* Mark the cluster as changed, so it will be written to disk when it
   is freed. Commit writes to disk immediately*/
void icalset_mark(icalset* set);
icalerrorenum icalset_commit(icalset* set); 

icalerrorenum icalset_add_component(icalset* set, icalcomponent* comp);
icalerrorenum icalset_remove_component(icalset* set, icalcomponent* comp);

int icalset_count_components(icalset* set,
			     icalcomponent_kind kind);

/* Restrict the component returned by icalset_first, _next to those
   that pass the gauge. _clear removes the gauge. */
icalerrorenum icalset_select(icalset* set, icalcomponent* gauge);
void icalset_clear_select(icalset* set);

/* Get a component by uid */
icalcomponent* icalset_fetch(icalset* set, const char* uid);
int icalset_has_uid(icalset* set, const char* uid);
icalcomponent* icalset_fetch_match(icalset* set, icalcomponent *c);

/* Modify components according to the MODIFY method of CAP. Works on
   the currently selected components. */
icalerrorenum icalset_modify(icalset* set, icalcomponent *oldc,
			       icalcomponent *newc);

/* Iterate through the components. If a guage has been defined, these
   will skip over components that do not pass the gauge */

icalcomponent* icalset_get_current_component(icalset* set);
icalcomponent* icalset_get_first_component(icalset* set);
icalcomponent* icalset_get_next_component(icalset* set);

#endif /* !ICALSET_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalfileset.h
 CREATOR: eric 23 December 1999


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

#ifndef ICALFILESET_H
#define ICALFILESET_H


typedef void icalfileset;


/* icalfileset
   icalfilesetfile
   icalfilesetdir
*/


icalfileset* icalfileset_new(const char* path);
void icalfileset_free(icalfileset* cluster);

const char* icalfileset_path(icalfileset* cluster);

/* Mark the cluster as changed, so it will be written to disk when it
   is freed. Commit writes to disk immediately. */
void icalfileset_mark(icalfileset* cluster);
icalerrorenum icalfileset_commit(icalfileset* cluster); 

icalerrorenum icalfileset_add_component(icalfileset* cluster,
					icalcomponent* child);

icalerrorenum icalfileset_remove_component(icalfileset* cluster,
					   icalcomponent* child);

int icalfileset_count_components(icalfileset* cluster,
				 icalcomponent_kind kind);

/* Restrict the component returned by icalfileset_first, _next to those
   that pass the gauge. _clear removes the gauge */
icalerrorenum icalfileset_select(icalfileset* store, icalcomponent* gauge);
void icalfileset_clear(icalfileset* store);

/* Get and search for a component by uid */
icalcomponent* icalfileset_fetch(icalfileset* cluster, const char* uid);
int icalfileset_has_uid(icalfileset* cluster, const char* uid);
icalcomponent* icalfileset_fetch_match(icalfileset* set, icalcomponent *c);


/* Modify components according to the MODIFY method of CAP. Works on
   the currently selected components. */
icalerrorenum icalfileset_modify(icalfileset* store, icalcomponent *oldcomp,
			       icalcomponent *newcomp);

/* Iterate through components. If a guage has been defined, these
   will skip over components that do not pass the gauge */

icalcomponent* icalfileset_get_current_component (icalfileset* cluster);
icalcomponent* icalfileset_get_first_component(icalfileset* cluster);
icalcomponent* icalfileset_get_next_component(icalfileset* cluster);
/* Return a reference to the internal component. You probably should
   not be using this. */

icalcomponent* icalfileset_get_component(icalfileset* cluster);


#endif /* !ICALFILESET_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icaldirset.h
 CREATOR: eric 28 November 1999


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

#ifndef ICALDIRSET_H
#define ICALDIRSET_H


/* icaldirset Routines for storing, fetching, and searching for ical
 * objects in a database */

typedef void icaldirset;


icaldirset* icaldirset_new(const char* path);

void icaldirset_free(icaldirset* store);

const char* icaldirset_path(icaldirset* store);

/* Mark the cluster as changed, so it will be written to disk when it
   is freed. Commit writes to disk immediately*/
void icaldirset_mark(icaldirset* store);
icalerrorenum icaldirset_commit(icaldirset* store); 

icalerrorenum icaldirset_add_component(icaldirset* store, icalcomponent* comp);
icalerrorenum icaldirset_remove_component(icaldirset* store, icalcomponent* comp);

int icaldirset_count_components(icaldirset* store,
			       icalcomponent_kind kind);

/* Restrict the component returned by icaldirset_first, _next to those
   that pass the gauge. _clear removes the gauge. */
icalerrorenum icaldirset_select(icaldirset* store, icalcomponent* gauge);
void icaldirset_clear(icaldirset* store);

/* Get a component by uid */
icalcomponent* icaldirset_fetch(icaldirset* store, const char* uid);
int icaldirset_has_uid(icaldirset* store, const char* uid);
icalcomponent* icaldirset_fetch_match(icaldirset* set, icalcomponent *c);

/* Modify components according to the MODIFY method of CAP. Works on
   the currently selected components. */
icalerrorenum icaldirset_modify(icaldirset* store, icalcomponent *oldc,
			       icalcomponent *newc);

/* Iterate through the components. If a guage has been defined, these
   will skip over components that do not pass the gauge */

icalcomponent* icaldirset_get_current_component(icaldirset* store);
icalcomponent* icaldirset_get_first_component(icaldirset* store);
icalcomponent* icaldirset_get_next_component(icaldirset* store);

#endif /* !ICALDIRSET_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalcalendar.h
 CREATOR: eric 23 December 1999


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

#ifndef ICALCALENDAR_H
#define ICALCALENDAR_H


/* icalcalendar
 * Routines for storing calendar data in a file system. The calendar 
 * has two icaldirsets, one for incoming components and one for booked
 * components. It also has interfaces to access the free/busy list
 * and a list of calendar properties */

typedef  void icalcalendar;

icalcalendar* icalcalendar_new(char* dir);

void icalcalendar_free(icalcalendar* calendar);

int icalcalendar_lock(icalcalendar* calendar);

int icalcalendar_unlock(icalcalendar* calendar);

int icalcalendar_islocked(icalcalendar* calendar);

int icalcalendar_ownlock(icalcalendar* calendar);

icalset* icalcalendar_get_booked(icalcalendar* calendar);

icalset* icalcalendar_get_incoming(icalcalendar* calendar);

icalset* icalcalendar_get_properties(icalcalendar* calendar);

icalset* icalcalendar_get_freebusy(icalcalendar* calendar);


#endif /* !ICALCALENDAR_H */



/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalgauge.h
 CREATOR: eric 23 December 1999


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

#ifndef ICALGAUGE_H
#define ICALGAUGE_H

typedef void icalgauge;

icalgauge* icalgauge_new_from_sql(char* sql);

void icalgauge_free(icalgauge* gauge);

char* icalgauge_as_sql(icalcomponent* gauge);

int icalgauge_test(icalcomponent* comp, icalcomponent* gaugecontainer);


#endif /* ICALGAUGE_H*/
/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalssutil.h
 CREATOR: eric 21 Aug 2000


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


 =========================================================================*/


/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalclassify.h
 CREATOR: eric 21 Aug 2000


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


 =========================================================================*/

#ifndef ICALCLASSIFY_H
#define ICALCLASSIFY_H



typedef enum icalclass {
    ICAL_NO_CLASS,
    ICAL_PUBLISH_NEW_CLASS,
    ICAL_PUBLISH_UPDATE_CLASS,
    ICAL_PUBLISH_FREEBUSY_CLASS,
    ICAL_REQUEST_NEW_CLASS,
    ICAL_REQUEST_UPDATE_CLASS,
    ICAL_REQUEST_RESCHEDULE_CLASS,
    ICAL_REQUEST_DELEGATE_CLASS,
    ICAL_REQUEST_NEW_ORGANIZER_CLASS,
    ICAL_REQUEST_FORWARD_CLASS,
    ICAL_REQUEST_STATUS_CLASS,
    ICAL_REQUEST_FREEBUSY_CLASS,
    ICAL_REPLY_ACCEPT_CLASS,
    ICAL_REPLY_DECLINE_CLASS,
    ICAL_REPLY_CRASHER_ACCEPT_CLASS,
    ICAL_REPLY_CRASHER_DECLINE_CLASS,
    ICAL_ADD_INSTANCE_CLASS,
    ICAL_CANCEL_EVENT_CLASS,
    ICAL_CANCEL_INSTANCE_CLASS,
    ICAL_CANCEL_ALL_CLASS,
    ICAL_REFRESH_CLASS,
    ICAL_COUNTER_CLASS,
    ICAL_DECLINECOUNTER_CLASS,
    ICAL_MALFORMED_CLASS, 
    ICAL_OBSOLETE_CLASS, /* 21 */
    ICAL_MISSEQUENCED_CLASS, /* 22 */
    ICAL_UNKNOWN_CLASS /* 23 */
} ical_class;

ical_class icalclassify(icalcomponent* c,icalcomponent* match, 
			      const char* user);

icalcomponent* icalclassify_find_overlaps(icalset* set, icalcomponent* comp);

#endif /* ICALCLASSIFY_H*/


				    


/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalspanlist.h
 CREATOR: eric 21 Aug 2000


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


 =========================================================================*/
#ifndef ICALSPANLIST_H
#define ICALSPANLIST_H


typedef void icalspanlist;

/* Make a free list from a set of component. Start and end should be in UTC */
icalspanlist* icalspanlist_new(icalset *set, 
				struct icaltimetype start,
				struct icaltimetype end);

void icalspanlist_free(icalspanlist* spl);

icalcomponent* icalspanlist_make_free_list(icalspanlist* sl);
icalcomponent* icalspanlist_make_busy_list(icalspanlist* sl);

/* Get first free or busy time after time t. all times are in UTC */
struct icalperiodtype icalspanlist_next_free_time(icalspanlist* sl,
						struct icaltimetype t);
struct icalperiodtype icalspanlist_next_busy_time(icalspanlist* sl,
						struct icaltimetype t);

void icalspanlist_dump(icalspanlist* s);

#endif
				    


/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalmessage.h
 CREATOR: eric 07 Nov 2000


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


 =========================================================================*/


#ifndef ICALMESSAGE_H
#define ICALMESSAGE_H


icalcomponent* icalmessage_new_accept_reply(icalcomponent* c, 
					    const char* user,
					    const char* msg);

icalcomponent* icalmessage_new_decline_reply(icalcomponent* c,
					    const char* user,
					    const char* msg);

/* New is modified version of old */
icalcomponent* icalmessage_new_counterpropose_reply(icalcomponent* old,
						    icalcomponent* new,
						    const char* user,
						    const char* msg);


icalcomponent* icalmessage_new_delegate_reply(icalcomponent* c,
					      const char* user,
					      const char* delegatee,
					      const char* msg);



icalcomponent* icalmessage_new_cancel_event(icalcomponent* c,
					    const char* user,
					    const char* msg);
icalcomponent* icalmessage_new_cancel_instance(icalcomponent* c,
					    const char* user,
					    const char* msg);
icalcomponent* icalmessage_new_cancel_all(icalcomponent* c,
					    const char* user,
					    const char* msg);


icalcomponent* icalmessage_new_error_reply(icalcomponent* c,
					   const char* user,
					   const char* msg,
					   const char* debug,
					   icalrequeststatus rs);


#endif /* ICALMESSAGE_H*/
