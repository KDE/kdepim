/* -*- Mode: C -*- */

/*======================================================================
  FILE: icalproperty.c
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

  The original code is icalproperty.c

======================================================================*/
/*#line 27 "icalproperty.c.in"*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "icalproperty.h"
#include "icalparameter.h"
#include "icalcomponent.h"
#include "pvl.h"
#include "icalenums.h"
#include "icalerror.h"
#include "icalmemory.h"
#include "icalparser.h"

#include <string.h> /* For icalmemory_strdup, rindex */
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h> /* for printf */
#include <stdarg.h> /* for va_list, va_start, etc. */
                                               
#define TMP_BUF_SIZE 1024

/* Private routines for icalproperty */
void icalvalue_set_parent(icalvalue* value,
			     icalproperty* property);
icalproperty* icalvalue_get_parent(icalvalue* value);

void icalparameter_set_parent(icalparameter* param,
			     icalproperty* property);
icalproperty* icalparameter_get_parent(icalparameter* value);


void icalproperty_set_x_name(icalproperty* prop, const char* name);

struct icalproperty_impl 
{
	char id[5];
	icalproperty_kind kind;
	char* x_name;
	pvl_list parameters;
	pvl_elem parameter_iterator;
	icalvalue* value;
	icalcomponent *parent;
};

void icalproperty_add_parameters(struct icalproperty_impl *prop,va_list args)
{

    void* vp;

    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;
  
    while((vp = va_arg(args, void*)) != 0) {

	if (icalvalue_isa_value(vp) != 0 ){
	} else if (icalparameter_isa_parameter(vp) != 0 ){

	    icalproperty_add_parameter((icalproperty*)impl,
				       (icalparameter*)vp);
	} else {
	    assert(0);
	}

    }
    
    
}


struct icalproperty_impl*
icalproperty_new_impl (icalproperty_kind kind)
{
    struct icalproperty_impl* prop;

    if ( ( prop = (struct icalproperty_impl*)
	   malloc(sizeof(struct icalproperty_impl))) == 0) {
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return 0;
    }
    
    strcpy(prop->id,"prop");

    prop->kind = kind;
    prop->parameters = pvl_newlist();
    prop->parameter_iterator = 0;
    prop->value = 0;
    prop->x_name = 0;
    prop->parent = 0;

    return prop;
}


icalproperty*
icalproperty_new (icalproperty_kind kind)
{
    if(kind == ICAL_NO_PROPERTY){
        return 0;
    }

    return (icalproperty*)icalproperty_new_impl(kind);
}


icalproperty*
icalproperty_new_clone(icalproperty* prop)
{
    struct icalproperty_impl *old = (struct icalproperty_impl*)prop;
    struct icalproperty_impl *new = icalproperty_new_impl(old->kind);
    pvl_elem p;

    icalerror_check_arg_rz((prop!=0),"Prop");
    icalerror_check_arg_rz((old!=0),"old");
    icalerror_check_arg_rz((new!=0),"new");

    if (old->value !=0) {
	new->value = icalvalue_new_clone(old->value);
    }

    if (old->x_name != 0) {

	new->x_name = icalmemory_strdup(old->x_name);
	
	if (new->x_name == 0) {
	    icalproperty_free(new);
	    icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	    return 0;
	}
    }

    for(p=pvl_head(old->parameters);p != 0; p = pvl_next(p)){
	icalparameter *param = icalparameter_new_clone(pvl_data(p));
	
	if (param == 0){
	    icalproperty_free(new);
	    icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	    return 0;
	}

	pvl_push(new->parameters,param);
    
    } 

    return new;

}

icalproperty* icalproperty_new_from_string(const char* str)
{

    size_t buf_size = 1024;
    char* buf = icalmemory_new_buffer(buf_size);
    char* buf_ptr = buf;  
    icalproperty *prop;
    icalcomponent *comp;
    int errors  = 0;

    icalerror_check_arg_rz( (str!=0),"str");

    /* Is this a HACK or a crafty reuse of code? */

    icalmemory_append_string(&buf, &buf_ptr, &buf_size, "BEGIN:VCALENDAR\n");
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, str);
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, "\n");    
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, "END:VCALENDAR\n");

    comp = icalparser_parse_string(buf);

    if(comp == 0){
        icalerror_set_errno(ICAL_PARSE_ERROR);
        return 0;
    }

    errors = icalcomponent_count_errors(comp);

    prop = icalcomponent_get_first_property(comp,ICAL_ANY_PROPERTY);

    icalcomponent_remove_property(comp,prop);

    icalcomponent_free(comp);
    free(buf);

    if(errors > 0){
        icalproperty_free(prop);
        return 0;
    } else {
        return prop;
    }
    
}

void
icalproperty_free (icalproperty* prop)
{
    struct icalproperty_impl *p;

    icalparameter* param;
    
    icalerror_check_arg_rv((prop!=0),"prop");

    p = (struct icalproperty_impl*)prop;

#ifdef ICAL_FREE_ON_LIST_IS_ERROR
    icalerror_assert( (p->parent ==0),"Tried to free a property that is still attached to a component. ");
    
#else
    if(p->parent !=0){
	return;
    }
#endif

    if (p->value != 0){
        icalvalue_set_parent(p->value,0);
	icalvalue_free(p->value);
    }
    
    while( (param = pvl_pop(p->parameters)) != 0){
	icalparameter_free(param);
    }
    
    pvl_free(p->parameters);
    
    if (p->x_name != 0) {
	free(p->x_name);
    }
    
    p->kind = ICAL_NO_PROPERTY;
    p->parameters = 0;
    p->parameter_iterator = 0;
    p->value = 0;
    p->x_name = 0;
    p->id[0] = 'X';
    
    free(p);

}


const char*
icalproperty_as_ical_string (icalproperty* prop)
{   
    icalparameter *param;

    /* Create new buffer that we can append names, parameters and a
       value to, and reallocate as needed. Later, this buffer will be
       copied to a icalmemory_tmp_buffer, which is managed internally
       by libical, so it can be given to the caller without fear of
       the caller forgetting to free it */

    const char* property_name = 0; 
    size_t buf_size = 1024;
    char* buf = icalmemory_new_buffer(buf_size);
    char* buf_ptr = buf;
    icalvalue* value;
    char *out_buf;

    char newline[] = "\n";

    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;
    
    icalerror_check_arg_rz( (prop!=0),"prop");


    /* Append property name */

    if (impl->kind == ICAL_X_PROPERTY && impl->x_name != 0){
	property_name = impl->x_name;
    } else {
	property_name = icalproperty_kind_to_string(impl->kind);
    }

    if (property_name == 0 ) {
	icalerror_warn("Got a property of an unknown kind.");
	icalmemory_free_buffer(buf);
	return 0;
	
    }


    icalmemory_append_string(&buf, &buf_ptr, &buf_size, property_name);
#ifdef KCAL_ADDLINEBREAKS
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);
#endif


    /* Determine what VALUE parameter to include. The VALUE parameters
       are ignored in the normal parameter printing ( the block after
       this one, so we need to do it here */
    {
	const char* kind_string = 0;

	icalparameter *orig_val_param
	    = icalproperty_get_first_parameter(prop,ICAL_VALUE_PARAMETER);

	icalvalue *value = icalproperty_get_value(impl);

	icalvalue_kind orig_kind = ICAL_NO_VALUE;

	icalvalue_kind this_kind = ICAL_NO_VALUE;

	icalvalue_kind default_kind 
	    =  icalproperty_kind_to_value_kind(impl->kind);

	if(orig_val_param){
	    orig_kind = (icalvalue_kind)icalparameter_get_value(orig_val_param);
	}

	if(value != 0){
	    this_kind = icalvalue_isa(value);
	}
	
	
	if(this_kind == default_kind &&
	   orig_kind != ICAL_NO_VALUE){
	    /* The kind is the default, so it does not need to be
               included, but do it anyway, since it was explicit in
               the property. But, use the default, not the one
               specified in the property */
	    
	    kind_string = icalvalue_kind_to_string(default_kind);

	} else if (this_kind != default_kind && this_kind !=  ICAL_NO_VALUE){
	    /* Not the default, so it must be specified */
	    kind_string = icalvalue_kind_to_string(this_kind);
	} else {
	    /* Don'tinclude the VALUE parameter at all */
	}

	if(kind_string!=0){
#ifdef KCAL_ADDLINEBREAKS
	    icalmemory_append_string(&buf, &buf_ptr, &buf_size, " ;");
#else
	    icalmemory_append_string(&buf, &buf_ptr, &buf_size, ";");
#endif
	    icalmemory_append_string(&buf, &buf_ptr, &buf_size, "VALUE=");
	    icalmemory_append_string(&buf, &buf_ptr, &buf_size, kind_string);
#ifdef KCAL_ADDLINEBREAKS
	    icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);
#endif
	}
	

    }

    /* Append parameters */
    for(param = icalproperty_get_first_parameter(prop,ICAL_ANY_PARAMETER);
	param != 0; 
	param = icalproperty_get_next_parameter(prop,ICAL_ANY_PARAMETER)) {

	char* kind_string = icalparameter_as_ical_string(param); 
	icalparameter_kind kind = icalparameter_isa(param);

	if(kind==ICAL_VALUE_PARAMETER){
	    continue;
	}

	if (kind_string == 0 ) {
	    char temp[TMP_BUF_SIZE];
	    snprintf(temp, TMP_BUF_SIZE,"Got a parameter of unknown kind in %s property",property_name);
	    icalerror_warn(temp);
	    continue;
	}

#ifdef KCAL_ADDLINEBREAKS
	icalmemory_append_string(&buf, &buf_ptr, &buf_size, " ;");
#else
	icalmemory_append_string(&buf, &buf_ptr, &buf_size, ";");
#endif
    	icalmemory_append_string(&buf, &buf_ptr, &buf_size, kind_string);
#ifdef KCAL_ADDLINEBREAKS
 	icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);
#endif

    }    

    /* Append value */

#ifdef KCAL_ADDLINEBREAKS
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, " :");
#else
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, ":");
#endif

    value = icalproperty_get_value(prop);

    if (value != 0){
	const char *str = icalvalue_as_ical_string(value);
	icalerror_assert((str !=0),"Could not get string representation of a value");
	icalmemory_append_string(&buf, &buf_ptr, &buf_size, str);
    } else {
	icalmemory_append_string(&buf, &buf_ptr, &buf_size,"ERROR: No Value"); 
	
    }
    
    icalmemory_append_string(&buf, &buf_ptr, &buf_size, newline);

    /* Now, copy the buffer to a tmp_buffer, which is safe to give to
       the caller without worring about de-allocating it. */

    
    out_buf = icalmemory_tmp_buffer(strlen(buf)+1);
    strcpy(out_buf, buf);

    icalmemory_free_buffer(buf);

    return out_buf;
}



icalproperty_kind
icalproperty_isa (icalproperty* property)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)property;

   if(property != 0){
       return p->kind;
   }

   return ICAL_NO_PROPERTY;
}

int
icalproperty_isa_property (void* property)
{
    struct icalproperty_impl *impl = (struct icalproperty_impl*)property;

    icalerror_check_arg_rz( (property!=0), "property");

    if (strcmp(impl->id,"prop") == 0) {
	return 1;
    } else {
	return 0;
    }
}


void
icalproperty_add_parameter (icalproperty* prop,icalparameter* parameter)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)prop;
    
   icalerror_check_arg_rv( (prop!=0),"prop");
   icalerror_check_arg_rv( (parameter!=0),"parameter");
    
   pvl_push(p->parameters, parameter);

}

void
icalproperty_set_parameter (icalproperty* prop,icalparameter* parameter)
{
    icalparameter_kind kind;
    
    icalerror_check_arg_rv( (prop!=0),"prop");
    icalerror_check_arg_rv( (parameter!=0),"parameter");

    kind = icalparameter_isa(parameter);

    icalproperty_remove_parameter(prop,kind);

    icalproperty_add_parameter(prop,parameter);
}

void icalproperty_set_parameter_from_string(icalproperty* prop,
                                            const char* name, const char* value)
{

    icalparameter_kind kind;
    icalparameter *param;

    icalerror_check_arg_rv( (prop!=0),"prop");
    icalerror_check_arg_rv( (name!=0),"name");
    icalerror_check_arg_rv( (value!=0),"value");
    
    kind = icalparameter_string_to_kind(name);

    if(kind == ICAL_NO_PARAMETER){
        icalerror_set_errno(ICAL_BADARG_ERROR);
        return;
    }

    param  = icalparameter_new_from_value_string(kind,value);

    if (param == 0){
        icalerror_set_errno(ICAL_BADARG_ERROR);
        return;
    }

    icalproperty_set_parameter(prop,param);

}

const char* icalproperty_get_parameter_as_string(icalproperty* prop,
                                                 const char* name)
{
    icalparameter_kind kind;
    icalparameter *param;
    char* str;
    char* pv;

    icalerror_check_arg_rz( (prop!=0),"prop");
    icalerror_check_arg_rz( (name!=0),"name");
    
    kind = icalparameter_string_to_kind(name);

    if(kind == ICAL_NO_PROPERTY){
        /* icalenum_string_to_parameter_kind will set icalerrno */
        return 0;
    }

    param = icalproperty_get_first_parameter(prop,kind);

    if (param == 0){
        return 0;
    }

    str = icalparameter_as_ical_string(param);

    pv = strchr(str,'=');

    if(pv == 0){
        icalerror_set_errno(ICAL_INTERNAL_ERROR);
        return 0;
    }

    return pv+1;

}

void
icalproperty_remove_parameter (icalproperty* prop, icalparameter_kind kind)
{
    pvl_elem p;     
    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;

    icalerror_check_arg_rv((prop!=0),"prop");
    
    for(p=pvl_head(impl->parameters);p != 0; p = pvl_next(p)){
	icalparameter* param = (icalparameter *)pvl_data (p);
        if (icalparameter_isa(param) == kind) {
            pvl_remove (impl->parameters, p);
            icalparameter_free (param);
            break;
        }
    }                       
}


int
icalproperty_count_parameters (icalproperty* prop)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)prop;

    if(prop != 0){
	return pvl_count(p->parameters);
    }

    icalerror_set_errno(ICAL_USAGE_ERROR);
    return -1;
}


icalparameter*
icalproperty_get_first_parameter (icalproperty* prop, icalparameter_kind kind)
{
   struct icalproperty_impl *p = (struct icalproperty_impl*)prop;

   icalerror_check_arg_rz( (prop!=0),"prop");
   
   p->parameter_iterator = pvl_head(p->parameters);

   if (p->parameter_iterator == 0) {
       return 0;
   }

   for( p->parameter_iterator = pvl_head(p->parameters);
	p->parameter_iterator !=0;
	p->parameter_iterator = pvl_next(p->parameter_iterator)){

       icalparameter *param = (icalparameter*)pvl_data(p->parameter_iterator);

       if(icalparameter_isa(param) == kind || kind == ICAL_ANY_PARAMETER){
	   return param;
       }
   }

   return 0;
}


icalparameter*
icalproperty_get_next_parameter (icalproperty* prop, icalparameter_kind kind)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)prop;
    
    icalerror_check_arg_rz( (prop!=0),"prop");
    
    if (p->parameter_iterator == 0) {
	return 0;
    }
    
    for( p->parameter_iterator = pvl_next(p->parameter_iterator);
	 p->parameter_iterator !=0;
	 p->parameter_iterator = pvl_next(p->parameter_iterator)){
	
	icalparameter *param = (icalparameter*)pvl_data(p->parameter_iterator);
	
	if(icalparameter_isa(param) == kind || kind == ICAL_ANY_PARAMETER){
	    return param;
	}
    }
    
    return 0;

}

void
icalproperty_set_value (icalproperty* prop, icalvalue* value)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)prop;

    icalerror_check_arg_rv((prop !=0),"prop");
    icalerror_check_arg_rv((value !=0),"value");
    
    if (p->value != 0){
	icalvalue_set_parent(p->value,0);
	icalvalue_free(p->value);
	p->value = 0;
    }

    p->value = value;
    
    icalvalue_set_parent(value,prop);
}


void icalproperty_set_value_from_string(icalproperty* prop,const char* str,
                                        const char* type)
{
    icalvalue *oval,*nval;
    icalvalue_kind kind = ICAL_NO_VALUE;

    icalerror_check_arg_rv( (prop!=0),"prop"); 
    icalerror_check_arg_rv( (str!=0),"str");
    icalerror_check_arg_rv( (type!=0),"type");
   
    if(strcmp(type,"NO")==0){
        /* Get the type from the value the property already has, if it exists */
        oval = icalproperty_get_value(prop);
        if(oval != 0){
            /* Use the existing value kind */
            kind  = icalvalue_isa(oval);
        } else {   
            /* Use the default kind for the property */
            kind = icalproperty_kind_to_value_kind(icalproperty_isa(prop));
        }
    } else {
        /* Use the given kind string */
        kind = icalvalue_string_to_kind(type);
    }

    if(kind == ICAL_NO_VALUE){
        icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        return;
    }

    nval = icalvalue_new_from_string(kind, str);

    if(nval == 0){
        /* icalvalue_new_from_string sets errno */
        assert(icalerrno != ICAL_NO_ERROR);
        return;
    }

    icalproperty_set_value(prop,nval);


}

icalvalue*
icalproperty_get_value (icalproperty* prop)
{
    struct icalproperty_impl *p = (struct icalproperty_impl*)prop;
    
    icalerror_check_arg_rz( (prop!=0),"prop");
    
    return p->value;
}

const char* icalproperty_get_value_as_string(icalproperty* prop)
{
    icalvalue *value;
    
    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;
    
    icalerror_check_arg_rz( (prop!=0),"prop");

    value = impl->value; 

    return icalvalue_as_ical_string(value);
}


void icalproperty_set_x_name(icalproperty* prop, const char* name)
{
    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;

    icalerror_check_arg_rv( (name!=0),"name");
    icalerror_check_arg_rv( (prop!=0),"prop");

    if (impl->x_name != 0) {
        free(impl->x_name);
    }

    impl->x_name = icalmemory_strdup(name);

    if(impl->x_name == 0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
    }

}
                              
const char* icalproperty_get_x_name(icalproperty* prop){

    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;

    icalerror_check_arg_rz( (prop!=0),"prop");

    return impl->x_name;
}


/* From Jonathan Yue <jonathan.yue@cp.net>    */
const char* icalproperty_get_name (icalproperty* prop)
{

    const char* property_name = 0;
    size_t buf_size = 256;
    char* buf = icalmemory_new_buffer(buf_size);
    char* buf_ptr = buf;  

    struct icalproperty_impl *impl = (struct icalproperty_impl*)prop;

    icalerror_check_arg_rz( (prop!=0),"prop");
 
    if (impl->kind == ICAL_X_PROPERTY && impl->x_name != 0){
        property_name = impl->x_name;
    } else {
        property_name = icalproperty_kind_to_string(impl->kind);
    }
 
    if (property_name == 0 ) {
        icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
        return 0;

    } else {
        /* _append_string will automatically grow the buffer if
           property_name is longer than the initial buffer size */
        icalmemory_append_string(&buf, &buf_ptr, &buf_size, property_name);
    }
 
    /* Add the buffer to the temporary buffer ring -- the caller will
       not have to free the memory. */
    icalmemory_add_tmp_buffer(buf);
 
    return buf;
}
                            



void icalproperty_set_parent(icalproperty* property,
			     icalcomponent* component)
{
    struct icalproperty_impl *impl = (struct icalproperty_impl*)property;

    icalerror_check_arg_rv( (property!=0),"property");
    
    impl->parent = component;
}

icalcomponent* icalproperty_get_parent(icalproperty* property)
{
    struct icalproperty_impl *impl = (struct icalproperty_impl*)property;
 
    icalerror_check_arg_rz( (property!=0),"property");

    return impl->parent;
}







/* Everything below this line is machine generated. Do not edit. */
