/*======================================================================
  FILE: icalcomponent.c
  CREATOR: eric 28 April 1999
  
  $Id$


 (C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalcomponent.c

======================================================================*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "icalcomponent.h"
#include "pvl.h" /* "Pointer-to-void list" */
#include "icalerror.h"
#include "icalmemory.h"
#include "icalenums.h"
#include "icaltime.h"
#include "icalduration.h"
#include "icalperiod.h"
#include "icalparser.h"

#include <stdlib.h>  /* for malloc */
#include <stdarg.h> /* for va_list, etc */
#include <errno.h>
#include <assert.h>
#include <stdio.h> /* for fprintf */

#define MAX_TMP 1024

struct icalcomponent_impl 
{
	char id[5];
	icalcomponent_kind kind;
	char* x_name;
	pvl_list properties;
	pvl_elem property_iterator;
	pvl_list components;
	pvl_elem component_iterator;
	icalcomponent* parent;
};

/* icalproperty functions that only components get to use */
void icalproperty_set_parent(icalproperty* property,
			     icalcomponent* component);
icalcomponent* icalproperty_get_parent(icalproperty* property);
void icalcomponent_add_children(struct icalcomponent_impl *impl,va_list args);
icalcomponent* icalcomponent_new_impl (icalcomponent_kind kind);
int icalcomponent_property_sorter(void *a, void *b);


void icalcomponent_add_children(struct icalcomponent_impl *impl,va_list args)
{
    void* vp;
    
    while((vp = va_arg(args, void*)) != 0) {

	assert (icalcomponent_isa_component(vp) != 0 ||
		icalproperty_isa_property(vp) != 0 ) ;

	if (icalcomponent_isa_component(vp) != 0 ){

	    icalcomponent_add_component((icalcomponent*)impl,
				       (icalcomponent*)vp);

	} else if (icalproperty_isa_property(vp) != 0 ){

	    icalcomponent_add_property((icalcomponent*)impl,
				       (icalproperty*)vp);
	}
    }    
}

icalcomponent*
icalcomponent_new_impl (icalcomponent_kind kind)
{
    struct icalcomponent_impl* comp;

    if ( ( comp = (struct icalcomponent_impl*)
	   malloc(sizeof(struct icalcomponent_impl))) == 0) {
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return 0;
    }
    
    strcpy(comp->id,"comp");

    comp->kind = kind;
    comp->properties = pvl_newlist();
    comp->property_iterator = 0;
    comp->components = pvl_newlist();
    comp->component_iterator = 0;
    comp->x_name = 0;
    comp->parent = 0;

    return comp;
}

icalcomponent*
icalcomponent_new (icalcomponent_kind kind)
{
   return (icalcomponent*)icalcomponent_new_impl(kind);
}

icalcomponent*
icalcomponent_vanew (icalcomponent_kind kind, ...)
{
   va_list args;

   struct icalcomponent_impl *impl = icalcomponent_new_impl(kind);

    if (impl == 0){
	return 0;
    }

   va_start(args,kind);
   icalcomponent_add_children(impl, args);
   va_end(args);

   return (icalcomponent*) impl;
}

icalcomponent* icalcomponent_new_from_string(char* str)
{
    return icalparser_parse_string(str);
}

icalcomponent* icalcomponent_new_clone(icalcomponent* component)
{
    struct icalcomponent_impl *old = (struct icalcomponent_impl*)component;
    struct icalcomponent_impl *new;
    icalproperty *p;
    icalcomponent *c;
    pvl_elem itr;

    icalerror_check_arg_rz( (component!=0), "component");

    new = icalcomponent_new_impl(old->kind);

    if (new == 0){
	return 0;
    }

    
    for( itr = pvl_head(old->properties);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	p = (icalproperty*)pvl_data(itr);
	icalcomponent_add_property(new,icalproperty_new_clone(p));
    }
   
   
    for( itr = pvl_head(old->components);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	c = (icalcomponent*)pvl_data(itr);
	icalcomponent_add_component(new,icalcomponent_new_clone(c));
    }

   return new;

}


void
icalcomponent_free (icalcomponent* component)
{
    icalproperty* prop;
    icalcomponent* comp;
    struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

    icalerror_check_arg_rv( (component!=0), "component");

#ifdef ICAL_FREE_ON_LIST_IS_ERROR
    icalerror_assert( (c->parent ==0),"Tried to free a component that is still attached to a parent component");
#else
    if(c->parent != 0){
	return;
    }
#endif

    if(component != 0 ){
       
       while( (prop=pvl_pop(c->properties)) != 0){
	   assert(prop != 0);
           icalproperty_set_parent(prop,0);
	   icalproperty_free(prop);
       }
       
       pvl_free(c->properties);

       while( (comp=pvl_data(pvl_head(c->components))) != 0){
	   assert(comp!=0);
	   icalcomponent_remove_component(component,comp);
	   icalcomponent_free(comp);
       }
       
       pvl_free(c->components);

	if (c->x_name != 0) {
	    free(c->x_name);
	}

	c->kind = ICAL_NO_COMPONENT;
	c->properties = 0;
	c->property_iterator = 0;
	c->components = 0;
	c->component_iterator = 0;
	c->x_name = 0;	
	c->id[0] = 'X';

	free(c);
    }
}

char*
icalcomponent_as_ical_string (icalcomponent* component)
{
   char* buf, *out_buf;
   char* tmp_buf;
   size_t buf_size = 1024;
   char* buf_ptr = 0;
    pvl_elem itr;
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;

#ifdef ICAL_UNIX_NEWLINE    
    char newline[] = "\n";
#else
    char newline[] = "\r\n";
#endif
   
   icalcomponent *c;
   icalproperty *p;
   icalcomponent_kind kind = icalcomponent_isa(component);

   const char* kind_string;

   buf = icalmemory_new_buffer(buf_size);
   buf_ptr = buf; 

   icalerror_check_arg_rz( (component!=0), "component");
   icalerror_check_arg_rz( (kind!=ICAL_NO_COMPONENT), "component kind is ICAL_NO_COMPONENT");
   
   kind_string  = icalcomponent_kind_to_string(kind);

   icalerror_check_arg_rz( (kind_string!=0),"Unknown kind of component");

   icalmemory_append_string(&buf, &buf_ptr, &buf_size, "BEGIN:");
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, kind_string);
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);
   


   for( itr = pvl_head(impl->properties);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	p = (icalproperty*)pvl_data(itr);
	
	icalerror_assert((p!=0),"Got a null property");
	tmp_buf = icalproperty_as_ical_string(p);
	
	icalmemory_append_string(&buf, &buf_ptr, &buf_size, tmp_buf);
    }
   
   
   for( itr = pvl_head(impl->components);
	itr != 0;
	itr = pvl_next(itr))
   {	
       c = (icalcomponent*)pvl_data(itr);
       
       tmp_buf = icalcomponent_as_ical_string(c);
       
       icalmemory_append_string(&buf, &buf_ptr, &buf_size, tmp_buf);
       
   }
   
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, "END:");
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, 
			    icalcomponent_kind_to_string(kind));
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);
   icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);

   out_buf = icalmemory_tmp_copy(buf);
   free(buf);

   return out_buf;
}


int
icalcomponent_is_valid (icalcomponent* component)
{
    struct icalcomponent_impl *impl = (struct icalcomponent_impl *)component;

	
    if ( (strcmp(impl->id,"comp") == 0) &&
	 impl->kind != ICAL_NO_COMPONENT){
	return 1;
    } else {
	return 0;
    }

}


icalcomponent_kind
icalcomponent_isa (icalcomponent* component)
{
    struct icalcomponent_impl *impl = (struct icalcomponent_impl *)component;
    icalerror_check_arg_rz( (component!=0), "component");

   if(component != 0)
   {
       return impl->kind;
   }

   return ICAL_NO_COMPONENT;
}


int
icalcomponent_isa_component (void* component)
{
    struct icalcomponent_impl *impl = (struct icalcomponent_impl *)component;

    icalerror_check_arg_rz( (component!=0), "component");

    if (strcmp(impl->id,"comp") == 0) {
	return 1;
    } else {
	return 0;
    }

}

int icalcomponent_property_sorter(void *a, void *b)
{
    icalproperty_kind kinda, kindb;
    const char *ksa, *ksb;

    kinda = icalproperty_isa((icalproperty*)a);
    kindb = icalproperty_isa((icalproperty*)b);

    ksa = icalproperty_kind_to_string(kinda);
    ksb = icalproperty_kind_to_string(kindb);

    return strcmp(ksa,ksb);
}


void
icalcomponent_add_property (icalcomponent* component, icalproperty* property)
{
    struct icalcomponent_impl *impl;

    icalerror_check_arg_rv( (component!=0), "component");
    icalerror_check_arg_rv( (property!=0), "property");

     impl = (struct icalcomponent_impl*)component;

    icalerror_assert( (!icalproperty_get_parent(property)),"The property has already been added to a component. Remove the property with icalcomponent_remove_property before calling icalcomponent_add_property");

    icalproperty_set_parent(property,component);

#ifdef ICAL_INSERT_ORDERED
    pvl_insert_ordered(impl->properties,
		       icalcomponent_property_sorter,property);
#else
    pvl_push(impl->properties,property);
#endif

}


void
icalcomponent_remove_property (icalcomponent* component, icalproperty* property)
{
    struct icalcomponent_impl *impl;
    pvl_elem itr, next_itr;
    struct icalproperty_impl *pimpl;

    icalerror_check_arg_rv( (component!=0), "component");
    icalerror_check_arg_rv( (property!=0), "property");
    
    impl = (struct icalcomponent_impl*)component;

    pimpl = (struct icalproperty_impl*)property;

    icalerror_assert( (icalproperty_get_parent(property)),"The property is not a member of a component");

    
    for( itr = pvl_head(impl->properties);
	 itr != 0;
	 itr = next_itr)
    {
	next_itr = pvl_next(itr);
	
	if( pvl_data(itr) == (void*)property ){

	   if (impl->property_iterator == itr){
	       impl->property_iterator = pvl_next(itr);
	   }

	   pvl_remove( impl->properties, itr); 
	  icalproperty_set_parent(property,0);
	}
    }	
}

int
icalcomponent_count_properties (icalcomponent* component, 
				icalproperty_kind kind)
{
    int count=0;
    pvl_elem itr;
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;

    icalerror_check_arg_rz( (component!=0), "component");

    for( itr = pvl_head(impl->properties);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	if(kind == icalproperty_isa((icalproperty*)pvl_data(itr)) ||
	    kind == ICAL_ANY_PROPERTY){
	    count++;
	}
    }


    return count;

}

icalproperty* icalcomponent_get_current_property (icalcomponent* component)
{

   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;
   icalerror_check_arg_rz( (component!=0),"component");

   if ((c->property_iterator==0)){
       return 0;
   }

   return (icalproperty*) pvl_data(c->property_iterator);

}

icalproperty*
icalcomponent_get_first_property (icalcomponent* component, icalproperty_kind kind)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;
   icalerror_check_arg_rz( (component!=0),"component");
  
   for( c->property_iterator = pvl_head(c->properties);
	c->property_iterator != 0;
	c->property_iterator = pvl_next(c->property_iterator)) {
	    
       icalproperty *p =  (icalproperty*) pvl_data(c->property_iterator);
	
	   if (icalproperty_isa(p) == kind || kind == ICAL_ANY_PROPERTY) {
	       
	       return p;
	   }
   }
   return 0;
}

icalproperty*
icalcomponent_get_next_property (icalcomponent* component, icalproperty_kind kind)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;
   icalerror_check_arg_rz( (component!=0),"component");

   if (c->property_iterator == 0){
       return 0;
   }

   for( c->property_iterator = pvl_next(c->property_iterator);
	c->property_iterator != 0;
	c->property_iterator = pvl_next(c->property_iterator)) {
	    
       icalproperty *p =  (icalproperty*) pvl_data(c->property_iterator);
	   
       if (icalproperty_isa(p) == kind || kind == ICAL_ANY_PROPERTY) {
	   
	   return p;
       }
   }

   return 0;
}


icalproperty**
icalcomponent_get_properties (icalcomponent* component, icalproperty_kind kind);


void
icalcomponent_add_component (icalcomponent* parent, icalcomponent* child)
{
    struct icalcomponent_impl *impl, *cimpl;

    icalerror_check_arg_rv( (parent!=0), "parent");
    icalerror_check_arg_rv( (child!=0), "child");
    
    impl = (struct icalcomponent_impl*)parent;
    cimpl = (struct icalcomponent_impl*)child;

    if (cimpl->parent !=0) {
        icalerror_set_errno(ICAL_USAGE_ERROR);
    }

    cimpl->parent = parent;

    pvl_push(impl->components,child);
}


void
icalcomponent_remove_component (icalcomponent* parent, icalcomponent* child)
{
   struct icalcomponent_impl *impl,*cimpl;
   pvl_elem itr, next_itr;

   icalerror_check_arg_rv( (parent!=0), "parent");
   icalerror_check_arg_rv( (child!=0), "child");
   
   impl = (struct icalcomponent_impl*)parent;
   cimpl = (struct icalcomponent_impl*)child;
   
   for( itr = pvl_head(impl->components);
	itr != 0;
	itr = next_itr)
   {
       next_itr = pvl_next(itr);
       
       if( pvl_data(itr) == (void*)child ){

	   if (impl->component_iterator == itr){
	       /* Don't let the current iterator become invalid */

	       /* HACK. The semantics for this are troubling. */
	       impl->component_iterator = 
		   pvl_next(impl->component_iterator);
	          
	   }
	   pvl_remove( impl->components, itr); 
	   cimpl->parent = 0;
	   break;
       }
   }	
}


int
icalcomponent_count_components (icalcomponent* component, 
				icalcomponent_kind kind)
{
    int count=0;
    pvl_elem itr;
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;

    icalerror_check_arg_rz( (component!=0), "component");

    for( itr = pvl_head(impl->components);
	 itr != 0;
	 itr = pvl_next(itr))
    {
	if(kind == icalcomponent_isa((icalcomponent*)pvl_data(itr)) ||
	    kind == ICAL_ANY_COMPONENT){
	    count++;
	}
    }

    return count;
}

icalcomponent*
icalcomponent_get_current_component(icalcomponent* component)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

   icalerror_check_arg_rz( (component!=0),"component");

   if (c->component_iterator == 0){
       return 0;
   }

   return (icalcomponent*) pvl_data(c->component_iterator);
}

icalcomponent*
icalcomponent_get_first_component (icalcomponent* component, 
				   icalcomponent_kind kind)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

   icalerror_check_arg_rz( (component!=0),"component");
  
   for( c->component_iterator = pvl_head(c->components);
	c->component_iterator != 0;
	c->component_iterator = pvl_next(c->component_iterator)) {
	    
       icalcomponent *p =  (icalcomponent*) pvl_data(c->component_iterator);
	
	   if (icalcomponent_isa(p) == kind || kind == ICAL_ANY_COMPONENT) {
	       
	       return p;
	   }
   }

   return 0;
}


icalcomponent*
icalcomponent_get_next_component (icalcomponent* component, icalcomponent_kind kind)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

   icalerror_check_arg_rz( (component!=0),"component");
  
   if (c->component_iterator == 0){
       return 0;
   }

   for( c->component_iterator = pvl_next(c->component_iterator);
	c->component_iterator != 0;
	c->component_iterator = pvl_next(c->component_iterator)) {
	    
       icalcomponent *p =  (icalcomponent*) pvl_data(c->component_iterator);
	
	   if (icalcomponent_isa(p) == kind || kind == ICAL_ANY_COMPONENT) {
	       
	       return p;
	   }
   }

   return 0;
}

icalcomponent* icalcomponent_get_first_real_component(icalcomponent *c)
{
    icalcomponent *comp;

    for(comp = icalcomponent_get_first_component(c,ICAL_ANY_COMPONENT);
	comp != 0;
	comp = icalcomponent_get_next_component(c,ICAL_ANY_COMPONENT)){

	icalcomponent_kind kind = icalcomponent_isa(comp);

	if(kind == ICAL_VEVENT_COMPONENT ||
	   kind == ICAL_VTODO_COMPONENT ||
	   kind == ICAL_VJOURNAL_COMPONENT ||
           kind == ICAL_VFREEBUSY_COMPONENT ){
	    return comp;
	}
    }
    return 0;
}

time_t icalcomponent_convert_time(icalproperty *p)
{
    struct icaltimetype sict;
    time_t convt;
    icalproperty *tzp;
        

    /* Though it says _dtstart, it will work for dtend too */
    sict = icalproperty_get_dtstart(p);

    tzp = icalproperty_get_first_parameter(p,ICAL_TZID_PARAMETER);

    if (sict.is_utc == 1 && tzp != 0){
	icalerror_warn("icalcomponent_get_span: component has a UTC DTSTART with a timezone specified ");
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	return 0; 
    }

    if(sict.is_utc == 1){
	/* _as_timet will use gmtime() to do the conversion */
	convt = icaltime_as_timet(sict);

#ifdef TEST_CONVERT_TIME
	printf("convert time: use as_timet:\n %s\n %s",
	       icalproperty_as_ical_string(p), ctime(&convt));
#endif

    } else if (sict.is_utc == 0 && tzp == 0 ) {
	time_t offset;

	/* _as_timet will use localtime() to do the conversion */
	convt = icaltime_as_timet(sict);
	offset = icaltime_utc_offset(sict,0);
	convt += offset;

#ifdef TEST_CONVERT_TIME
	printf("convert time: use as_timet and adjust:\n %s\n %s",
	       icalproperty_as_ical_string(p), ctime(&convt));
#endif
    } else {
	/* Convert the time to UTC for the named timezone*/
	const char* timezone = icalparameter_get_tzid(tzp);
	convt = icaltime_as_timet(icaltime_as_utc(sict,timezone));

#ifdef TEST_CONVERT_TIME
	printf("convert time: use _as_utc:\n %s\n %s",
	       icalproperty_as_ical_string(p), ctime(&convt));
#endif
    }	    

    return convt;
}
struct icaltime_span icalcomponent_get_span(icalcomponent* comp)
{
    icalcomponent *inner;
    icalproperty *p, *duration;
    icalcomponent_kind kind;
    struct icaltime_span span;
    struct icaltimetype start;

    span.start = 0;
    span.end = 0;
    span.is_busy= 1;

    /* initial Error checking */

/*    icalerror_check_arg_rz( (comp!=0),"comp");*/

    kind  = icalcomponent_isa(comp);

    if(kind == ICAL_VCALENDAR_COMPONENT){
	inner = icalcomponent_get_first_real_component(comp);

	/* Maybe there is a VTIMEZONE in there */
	if (inner == 0){
	    inner = icalcomponent_get_first_component(comp,
			       ICAL_VTIMEZONE_COMPONENT);
	}

    } else {
	inner = comp;
    }

    if (inner == 0){
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	/*icalerror_warn("icalcomponent_get_span: no component specified, or empty VCALENDAR component");*/
	return span; 
    }
    
    kind  = icalcomponent_isa(inner);

    if( !( kind == ICAL_VEVENT_COMPONENT ||
	   kind == ICAL_VJOURNAL_COMPONENT ||
	   kind == ICAL_VTODO_COMPONENT ||	   
	   kind == ICAL_VFREEBUSY_COMPONENT )) {
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	/*icalerror_warn("icalcomponent_get_span: no component specified, or empty VCALENDAR component");*/
	return span; 
	
    }



    /* Get to work. starting with DTSTART */

    p = icalcomponent_get_first_property(inner, ICAL_DTSTART_PROPERTY);

    if (p ==0 ) {
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	/*icalerror_warn("icalcomponent_get_span: component has no DTSTART time");*/
	return span; 
    }


    start = icalproperty_get_dtstart(p);

    icalerror_clear_errno();

    span.start = icalcomponent_convert_time(p);

#ifdef TEST_CONVERT_TIME
    printf("convert time:\n %s %s",
	   icalproperty_as_ical_string(p), ctime(&span.start));
#endif

    if(icalerrno != ICAL_NO_ERROR){
	span.start = 0;
	return span;
    }

    /* The end time could be specified as either a DTEND or a DURATION */
    p = icalcomponent_get_first_property(inner, ICAL_DTEND_PROPERTY);
    duration = icalcomponent_get_first_property(inner, ICAL_DURATION_PROPERTY);

    if (p==0 && duration == 0 && start.is_date != 1) {
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	/*icalerror_warn("icalcomponent_get_span: component has neither DTEND nor DURATION time");*/
	span.start = 0;
	return span; 
    } 

    if (p!=0){
	span.end = icalcomponent_convert_time(p);
    } else if (start.is_date == 1) {
	/* Duration is all day */
	span.end = span.start + 60*60*24;
    } else {
	/* Use the duration */
	struct icaldurationtype dur;
	time_t durt;
	
	
	dur = icalproperty_get_duration(duration);

	durt = icaldurationtype_as_int(dur);
	span.end = span.start+durt;
    }

    return span;

}


int icalcomponent_count_errors(icalcomponent* component)
{
    int errors = 0;
    icalproperty *p;
    icalcomponent *c;
    pvl_elem itr;
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;

    for( itr = pvl_head(impl->properties);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	p = (icalproperty*)pvl_data(itr);
	
	if(icalproperty_isa(p) == ICAL_XLICERROR_PROPERTY)
	{
	    errors++;
	}
    }


    for( itr = pvl_head(impl->components);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	c = (icalcomponent*)pvl_data(itr);
	
	errors += icalcomponent_count_errors(c);
	
    }

    return errors;
}


void icalcomponent_strip_errors(icalcomponent* component)
{
    icalproperty *p;
    icalcomponent *c;
    pvl_elem itr, next_itr;
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;

   for( itr = pvl_head(impl->properties);
	 itr != 0;
	 itr = next_itr)
    {	
	p = (icalproperty*)pvl_data(itr);
	next_itr = pvl_next(itr);

	if(icalproperty_isa(p) == ICAL_XLICERROR_PROPERTY)
	{
	    icalcomponent_remove_property(component,p);
	}
    }
    
    for( itr = pvl_head(impl->components);
	 itr != 0;
	 itr = pvl_next(itr))
    {	
	c = (icalcomponent*)pvl_data(itr);
	icalcomponent_strip_errors(c);
    }
}

/* Hack. This will change the state of the iterators */
void icalcomponent_convert_errors(icalcomponent* component)
{
    icalproperty *p, *next_p;
    icalcomponent *c;

    for(p = icalcomponent_get_first_property(component,ICAL_ANY_PROPERTY);
	p != 0;
	p = next_p){
	
	next_p = icalcomponent_get_next_property(component,ICAL_ANY_PROPERTY);

	if(icalproperty_isa(p) == ICAL_XLICERROR_PROPERTY)
	{
	    struct icalreqstattype rst;
	    icalparameter *param  = icalproperty_get_first_parameter
		(p,ICAL_XLICERRORTYPE_PARAMETER);

	    rst.code = ICAL_UNKNOWN_STATUS;
	    rst.desc = 0;

	    switch(icalparameter_get_xlicerrortype(param)){

		case  ICAL_XLICERRORTYPE_PARAMETERNAMEPARSEERROR: {
		    rst.code = ICAL_3_2_INVPARAM_STATUS;
		    break;
		}
		case  ICAL_XLICERRORTYPE_PARAMETERVALUEPARSEERROR: {
		    rst.code = ICAL_3_3_INVPARAMVAL_STATUS;
		    break;
		}
		case  ICAL_XLICERRORTYPE_PROPERTYPARSEERROR: {		    
		    rst.code = ICAL_3_0_INVPROPNAME_STATUS;
		    break;
		}
		case  ICAL_XLICERRORTYPE_VALUEPARSEERROR: {
		    rst.code = ICAL_3_1_INVPROPVAL_STATUS;
		    break;
		}
		case  ICAL_XLICERRORTYPE_COMPONENTPARSEERROR: {
		    rst.code = ICAL_3_4_INVCOMP_STATUS;
		    break;
		}

		default: {
		}
	    }
	    if (rst.code != ICAL_UNKNOWN_STATUS){
		
		rst.debug = icalproperty_get_xlicerror(p);
		icalcomponent_add_property(component,
					   icalproperty_new_requeststatus(rst));
		
		icalcomponent_remove_property(component,p);
	    }
	}
    }

    for(c = icalcomponent_get_first_component(component,ICAL_ANY_COMPONENT);
	c != 0;
	c = icalcomponent_get_next_component(component,ICAL_ANY_COMPONENT)){
	
	icalcomponent_convert_errors(c);
    }
}


icalcomponent* icalcomponent_get_parent(icalcomponent* component)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

   return c->parent;
}

void icalcomponent_set_parent(icalcomponent* component, icalcomponent* parent)
{
   struct icalcomponent_impl *c = (struct icalcomponent_impl*)component;

   c->parent = parent;
}

icalcompiter icalcompiter_null = {ICAL_NO_COMPONENT,0};


struct icalcomponent_kind_map {
	icalcomponent_kind kind;
	char name[20];
};

  

static struct icalcomponent_kind_map component_map[] = 
{
    { ICAL_VEVENT_COMPONENT, "VEVENT" },
    { ICAL_VTODO_COMPONENT, "VTODO" },
    { ICAL_VJOURNAL_COMPONENT, "VJOURNAL" },
    { ICAL_VCALENDAR_COMPONENT, "VCALENDAR" },
    { ICAL_VFREEBUSY_COMPONENT, "VFREEBUSY" },
    { ICAL_VTIMEZONE_COMPONENT, "VTIMEZONE" },
    { ICAL_VALARM_COMPONENT, "VALARM" },
    { ICAL_XSTANDARD_COMPONENT, "STANDARD" }, /*These are part of RFC2445 */
    { ICAL_XDAYLIGHT_COMPONENT, "DAYLIGHT" }, /*but are not really components*/
    { ICAL_X_COMPONENT, "X" },
    { ICAL_VSCHEDULE_COMPONENT, "SCHEDULE" },

    /* CAP components */
    { ICAL_VQUERY_COMPONENT, "VQUERY" },  
    { ICAL_VCAR_COMPONENT, "VCAR" },  
    { ICAL_VCOMMAND_COMPONENT, "VCOMMAND" },  

    /* libical private components */
    { ICAL_XLICINVALID_COMPONENT, "X-LIC-UNKNOWN" },  
    { ICAL_XLICMIMEPART_COMPONENT, "X-LIC-MIME-PART" },  
    { ICAL_ANY_COMPONENT, "ANY" },  
    { ICAL_XROOT_COMPONENT, "XROOT" },  

    /* End of list */
    { ICAL_NO_COMPONENT, "" },
};



const char* icalcomponent_kind_to_string(icalcomponent_kind kind)
{
    int i;

    for (i=0; component_map[i].kind != ICAL_NO_COMPONENT; i++) {
	if (component_map[i].kind == kind) {
	    return component_map[i].name;
	}
    }

    return 0;

}

icalcomponent_kind icalcomponent_string_to_kind(const char* string)
{
    int i;

    if (string ==0 ) { 
	return ICAL_NO_COMPONENT;
    }

    for (i=0; component_map[i].kind  != ICAL_NO_COMPONENT; i++) {
	if (strcmp(component_map[i].name, string) == 0) {
	    return component_map[i].kind;
	}
    }

    return ICAL_NO_COMPONENT;
}



icalcompiter 
icalcomponent_begin_component(icalcomponent* component,icalcomponent_kind kind)
{
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;
    icalcompiter itr;
    pvl_elem i;

    itr.kind = kind;

    icalerror_check_arg_re( (component!=0),"component",icalcompiter_null);

    for( i = pvl_head(impl->components); i != 0; i = pvl_next(itr.iter)) {
	
	icalcomponent *c =  (icalcomponent*) pvl_data(i);
	
	if (icalcomponent_isa(c) == kind || kind == ICAL_ANY_COMPONENT) {
	    
	    itr.iter = i;

	    return itr;
	}
    }

    return icalcompiter_null;
}

icalcompiter
icalcomponent_end_component(icalcomponent* component,icalcomponent_kind kind)
{
    struct icalcomponent_impl *impl = (struct icalcomponent_impl*)component;
    icalcompiter itr; 
    pvl_elem i;

    itr.kind = kind;

    icalerror_check_arg_re( (component!=0),"component",icalcompiter_null);

    for( i = pvl_tail(impl->components); i != 0; i = pvl_prior(i)) {
	
	icalcomponent *c =  (icalcomponent*) pvl_data(i);
	
	if (icalcomponent_isa(c) == kind || kind == ICAL_ANY_COMPONENT) {
	    
	    itr.iter = pvl_next(i);

	    return itr;
	}
    }

    return icalcompiter_null;;
}


icalcomponent* icalcompiter_next(icalcompiter* i)
{
   if (i->iter == 0){
       return 0;
   }

   icalerror_check_arg_rz( (i!=0),"i");

   for( i->iter = pvl_next(i->iter);
	i->iter != 0;
	i->iter = pvl_next(i->iter)) {
	    
       icalcomponent *c =  (icalcomponent*) pvl_data(i->iter);
	
	   if (icalcomponent_isa(c) == i->kind 
	       || i->kind == ICAL_ANY_COMPONENT) {
	       
	       return icalcompiter_deref(i);;
	   }
   }

   return 0;

}

icalcomponent* icalcompiter_prior(icalcompiter* i)
{
   if (i->iter == 0){
       return 0;
   }

   for( i->iter = pvl_prior(i->iter);
	i->iter != 0;
	i->iter = pvl_prior(i->iter)) {
	    
       icalcomponent *c =  (icalcomponent*) pvl_data(i->iter);
	
	   if (icalcomponent_isa(c) == i->kind 
	       || i->kind == ICAL_ANY_COMPONENT) {
	       
	       return icalcompiter_deref(i);;
	   }
   }

   return 0;

}
icalcomponent* icalcompiter_deref(icalcompiter* i)
{
    if(i->iter ==0){
	return 0;
    }

    return pvl_data(i->iter);
}

icalcomponent* icalcomponent_get_inner(icalcomponent* comp)
{
    if (icalcomponent_isa(comp) == ICAL_VCALENDAR_COMPONENT){
	return icalcomponent_get_first_real_component(comp);
    } else {
	return comp;
    }
}


void icalcomponent_set_dtstart(icalcomponent* comp, struct icaltimetype v)
{

    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner, ICAL_DTSTART_PROPERTY);


    if (prop == 0){
	prop = icalproperty_new_dtstart(v);
	icalcomponent_add_property(inner, prop);
    }
    
    icalproperty_set_dtstart(prop,v);

}


struct icaltimetype icalcomponent_get_dtstart(icalcomponent* comp)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner,ICAL_DTSTART_PROPERTY);

    if (prop == 0){
	return icaltime_null_time();
    }
    
    return icalproperty_get_dtstart(prop);
}


struct icaltimetype icalcomponent_get_dtend(icalcomponent* comp)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 

    icalproperty *end_prop 
	= icalcomponent_get_first_property(inner,ICAL_DTEND_PROPERTY);

    icalproperty *dur_prop 
	= icalcomponent_get_first_property(inner, ICAL_DURATION_PROPERTY);


    if( end_prop == 0 && dur_prop == 0){
	return icaltime_null_time();
    } else if ( end_prop != 0) {
	return icalproperty_get_dtend(end_prop);
    } else if ( dur_prop != 0) { 
	
	struct icaltimetype start = 
	    icalcomponent_get_dtstart(inner);
	struct icaldurationtype duration = 
	    icalproperty_get_duration(dur_prop);
	
	struct icaltimetype end = icaltime_add(start,duration);

	return end;

    } else {
	/* Error, both duration and dtend have been specified */
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	return icaltime_null_time();

    }
    
}


void icalcomponent_set_dtend(icalcomponent* comp, struct icaltimetype v)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 

    icalproperty *end_prop 
	= icalcomponent_get_first_property(inner,ICAL_DTEND_PROPERTY);

    icalproperty *dur_prop 
	= icalcomponent_get_first_property(inner,ICAL_DURATION_PROPERTY);


    if( end_prop == 0 && dur_prop == 0){
	end_prop = icalproperty_new_dtend(v);
	icalcomponent_add_property(inner,end_prop);
    } else if ( end_prop != 0) {
	icalproperty_set_dtend(end_prop,v);
    } else if ( dur_prop != 0) { 
	struct icaltimetype start = 
	    icalcomponent_get_dtstart(inner);

	struct icaltimetype end = 
	    icalcomponent_get_dtend(inner);

	struct icaldurationtype dur 
	    = icaltime_subtract(end,start);

	icalproperty_set_duration(dur_prop,dur);

    } else {
	/* Error, both duration and dtend have been specified */
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
    }
}

void icalcomponent_set_duration(icalcomponent* comp, 
				struct icaldurationtype v)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 

    icalproperty *end_prop 
	= icalcomponent_get_first_property(inner,ICAL_DTEND_PROPERTY);

    icalproperty *dur_prop 
	= icalcomponent_get_first_property(inner,ICAL_DURATION_PROPERTY);


    if( end_prop == 0 && dur_prop == 0){
	dur_prop = icalproperty_new_duration(v);
	icalcomponent_add_property(inner, dur_prop);
    } else if ( end_prop != 0) {
	struct icaltimetype start = 
	    icalcomponent_get_dtstart(inner);
	
	struct icaltimetype new_end = icaltime_add(start,v);

	icalproperty_set_dtend(end_prop,new_end);

    } else if ( dur_prop != 0) { 
	icalproperty_set_duration(dur_prop,v);
    } else {
	/* Error, both duration and dtend have been specified */
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
    }
}

struct icaldurationtype icalcomponent_get_duration(icalcomponent* comp)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 

    icalproperty *end_prop 
	= icalcomponent_get_first_property(inner,ICAL_DTEND_PROPERTY);

    icalproperty *dur_prop 
	= icalcomponent_get_first_property(inner,ICAL_DURATION_PROPERTY);

    struct icaldurationtype null_duration;
    memset(&null_duration,0,sizeof(struct icaldurationtype));


    if( end_prop == 0 && dur_prop == 0){
	return null_duration;
    } else if ( end_prop != 0) {
	struct icaltimetype start = 
	    icalcomponent_get_dtstart(inner);
	time_t startt = icaltime_as_timet(start);

	struct icaltimetype end = 
	    icalcomponent_get_dtend(inner);
	time_t endt = icaltime_as_timet(end);
	
	return icaldurationtype_from_int(endt-startt);
    } else if ( dur_prop != 0) { 
	return icalproperty_get_duration(dur_prop);
    } else {
	/* Error, both duration and dtend have been specified */
	icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
	return null_duration;
    }
}

void icalcomponent_set_method(icalcomponent* comp, icalproperty_method method)
{
    icalproperty *prop 
	= icalcomponent_get_first_property(comp, ICAL_METHOD_PROPERTY);


    if (prop == 0){
	prop = icalproperty_new_method(method);
	icalcomponent_add_property(comp, prop);
    }
    
    icalproperty_set_method(prop,method);

}

icalproperty_method icalcomponent_get_method(icalcomponent* comp)
{
    icalproperty *prop 
	= icalcomponent_get_first_property(comp,ICAL_METHOD_PROPERTY);

    if (prop == 0){
	return ICAL_METHOD_NONE;
    }
    
    return icalproperty_get_method(prop);
}

void icalcomponent_set_dtstamp(icalcomponent* comp, struct icaltimetype v)
{

    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner, ICAL_DTSTAMP_PROPERTY);


    if (prop == 0){
	prop = icalproperty_new_dtstamp(v);
	icalcomponent_add_property(inner, prop);
    }
    
    icalproperty_set_dtstamp(prop,v);

}


struct icaltimetype icalcomponent_get_dtstamp(icalcomponent* comp)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner,ICAL_DTSTAMP_PROPERTY);

    if (prop == 0){
	return icaltime_null_time();
    }
    
    return icalproperty_get_dtstamp(prop);
}


void icalcomponent_set_summary(icalcomponent* comp, const char* v)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner, ICAL_SUMMARY_PROPERTY);

    if (prop == 0){
	prop = icalproperty_new_summary(v);
	icalcomponent_add_property(inner, prop);
    }
    
    icalproperty_set_summary(prop,v);
}


const char* icalcomponent_get_summary(icalcomponent* comp)
{
    icalcomponent *inner = icalcomponent_get_inner(comp); 
    icalproperty *prop 
	= icalcomponent_get_first_property(inner,ICAL_SUMMARY_PROPERTY);

    if (prop == 0){
	return 0;
    }
    
    return icalproperty_get_summary(prop);

}

void icalcomponent_set_comment(icalcomponent* comp, const char* v);
const char* icalcomponent_get_comment(icalcomponent* comp);

void icalcomponent_set_uid(icalcomponent* comp, const char* v);
const char* icalcomponent_get_uid(icalcomponent* comp);

void icalcomponent_set_recurrenceid(icalcomponent* comp, 
				    struct icaltimetype v);
struct icaltimetype icalcomponent_get_recurrenceid(icalcomponent* comp);




icalcomponent* icalcomponent_new_vcalendar()
{
    return icalcomponent_new(ICAL_VCALENDAR_COMPONENT);
}
icalcomponent* icalcomponent_new_vevent()
{
    return icalcomponent_new(ICAL_VEVENT_COMPONENT);
}
icalcomponent* icalcomponent_new_vtodo()
{
    return icalcomponent_new(ICAL_VTODO_COMPONENT);
}
icalcomponent* icalcomponent_new_vjournal()
{
    return icalcomponent_new(ICAL_VJOURNAL_COMPONENT);
}
icalcomponent* icalcomponent_new_valarm()
{
    return icalcomponent_new(ICAL_VALARM_COMPONENT);
}
icalcomponent* icalcomponent_new_vfreebusy()
{
    return icalcomponent_new(ICAL_VFREEBUSY_COMPONENT);
}
icalcomponent* icalcomponent_new_vtimezone()
{
    return icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);
}
icalcomponent* icalcomponent_new_xstandard()
{
    return icalcomponent_new(ICAL_XSTANDARD_COMPONENT);
}
icalcomponent* icalcomponent_new_xdaylight()
{
    return icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);
}
