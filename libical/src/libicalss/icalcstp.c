/* -*- Mode: C -*-
    ======================================================================
    FILE: icalcstps.c
    CREATOR: ebusboom 23 Jun 2000
  
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ical.h"
#include "icalcstp.h"
#include "pvl.h" 

#include <sys/types.h> /* For send(), others */
#include <sys/socket.h>  /* For send(), others. */
#include <unistd.h> /* For alarm */
#include <errno.h>

enum cstps_state {
    NO_STATE,
    CONNECTED,
    AUTHENTICATED,
    IDENTIFIED,
    DISCONNECTED,
    RECEIVE,
};

struct icalcstps_impl {
	int timeout;
	icalparser *parser;
	enum cstps_state major_state;
	struct icalcstps_stubs stubs;
};


enum cstp_command {
    ABORT,
    AUTHENTICATE,
    CAPABILITY,
    CONTINUE,
    EXPANDCALID,
    IDENTIFY,
    DISCONNECT,
    SENDDATA,
    STARTTLS,
    EXPANDUPN,
    COMMAND_COMPLETE,
    UNKNOWN
};

struct command_map {
	enum cstp_command command;
	char *str;
} command_map[] = 
{
    {ABORT,"ABORT"},
    {AUTHENTICATE,"AUTHENTICATE"},
    {CAPABILITY,"CAPABILITY"},
    {CONTINUE,"CONTINUE"},
    {EXPANDCALID,"EXPANDCALID"},
    {IDENTIFY,"IDENTIFY"},
    {DISCONNECT,"DISCONNECT"},
    {SENDDATA,"SENDDATA"},
    {STARTTLS,"STARTTLS"},
    {EXPANDUPN,"EXPANDUPN"},
    {UNKNOWN,"UNKNOWN"}
};



/* This state machine is a Mealy-type: actions occur on the
   transitions, not in the states.
   
   Here is the state machine diagram from the CAP draft:


        STARTTLS /
        CAPABILITY
       +-------+
       |       |                       +---------------+
       |   +-----------+ AUTHENTICATE  |               |
       +-->| Connected |-------------->| Authenticated |
           +-----------+               |               |
             |                         +---------------+
             |                              |
             |                              |
             |                              |
             |                              |       +-----+ STARTTLS /
             |                              V       |     | CAPABILITY /
             |                         +---------------+  | IDENTIFY
             |                         |               |<-+
             |                         |   Identified  |<----+
             |                +--------|               |     |
             |                |        +---------------+     | command
             |                |             |                | completes
             V                |DISCONNECT   |                |
           +--------------+   |             |SENDDATA        |
           | Disconnected |<--+             |                |
           +--------------+                 |                | ABORT
                     A                      |                |
                     |                      V                |
                     |     DISCONNECT     +---------------+  |
                     +--------------------|    Receive    |--+
                                          |               |<--+
                                          +---------------+   |
                                                         |    | CONTINUTE
                                                         +----+

   In this implmenetation, the transition from CONNECTED to IDENTIFIED
   is non-standard. The spec specifies that on the ATHENTICATE
   command, the machine transitions from CONNECTED to AUTHENTICATED,
   and then immediately goes to IDENTIFIED. This makes AUTHENTICATED a
   useless state, so I removed it */

struct state_table {
	enum cstps_state major_state;
	enum cstp_command command;
	void (*action)();
	enum cstps_state next_state;

} server_state_table[] = 
{
    { CONNECTED, CAPABILITY , 0, CONNECTED},
    { CONNECTED, AUTHENTICATE , 0,  IDENTIFIED}, /* Non-standard */
    { IDENTIFIED, STARTTLS, 0, IDENTIFIED},
    { IDENTIFIED, IDENTIFY, 0, IDENTIFIED},
    { IDENTIFIED, CAPABILITY, 0, IDENTIFIED},
    { IDENTIFIED, SENDDATA, 0, RECEIVE},
    { IDENTIFIED, DISCONNECT, 0, DISCONNECTED},
    { DISCONNECTED, 0, 0, 0},
    { RECEIVE,  DISCONNECT, 0, DISCONNECTED},
    { RECEIVE,  CONTINUE, 0, RECEIVE},
    { RECEIVE,  ABORT , 0, IDENTIFIED},
    { RECEIVE,  COMMAND_COMPLETE , 0, IDENTIFIED}
};


/**********************************************************************/



icalcstps* icalcstps_new(struct icalcstps_stubs stubs)
{
    struct icalcstps_impl* impl;

    if ( ( impl = (struct icalcstps_impl*)
	   malloc(sizeof(struct icalcstps_impl))) == 0) {
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return 0;
    }

    impl->stubs = stubs;
    impl->timeout = 10;

    return (icalcstps*)impl;

}

void icalcstps_free(icalcstps* cstp);

int icalcstps_set_timeout(icalcstps* cstp, int sec) 
{
    struct icalcstps_impl *impl = (struct icalcstps_impl *) cstp;

    icalerror_check_arg_rz( (cstp!=0), "cstp");

    impl->timeout = sec;

    return sec;
}

typedef struct icalcstps_response {	
	icalrequeststatus code;
	char caluid[1024];
	void* result;
} icalcstps_response;

int line_is_command(char* line);
int line_is_response(char* line);
int line_is_endofdata(char* line);
int line_is_mime(char* line);

icalerrorenum prep_abort(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_authenticate(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_capability(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_calidexpand(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_continue(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_disconnect(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_identify(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_starttls(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_upnexpand(struct icalcstps_impl* impl, char* data)
{}
icalerrorenum prep_sendata(struct icalcstps_impl* impl, char* data)
{}

char* icalcstps_process_incoming(icalcstps* cstp, char* input)
{
    struct icalcstps_impl *impl = (struct icalcstps_impl *) cstp;
    char *i;
    char *cmd_or_resp;
    char *data;
    char *input_cpy;
    icalerrorenum error;

    icalerror_check_arg_rz(cstp !=0,"cstp");
    icalerror_check_arg_rz(input !=0,"input");

    if ((input_cpy = (char*)strdup(input)) == 0){
	icalerror_set_errno(ICAL_NEWFAILED_ERROR);
	return 0;
    }

    i = (char*)index(" ",input_cpy);

    cmd_or_resp = input_cpy;

    if (i != 0){
	*i = '\0';
	data = ++i;
    } else {
	data = 0;
    }

    printf("cmd: %s\n",cmd_or_resp);
    printf("data: %s\n",data);
	
    /* extract the command, look up in the state table, and dispatch
       to the proper handler */    

    if(strcmp(cmd_or_resp,"ABORT") == 0){
	error = prep_abort(impl,data);	
    } else if(strcmp(cmd_or_resp,"AUTHENTICATE") == 0){
	error = prep_authenticate(impl,data);
    } else if(strcmp(cmd_or_resp,"CAPABILITY") == 0){
	error = prep_capability(impl,data);
    } else if(strcmp(cmd_or_resp,"CALIDEXPAND") == 0){
	error = prep_calidexpand(impl,data);
    } else if(strcmp(cmd_or_resp,"CONTINUE") == 0){
	error = prep_continue(impl,data);
    } else if(strcmp(cmd_or_resp,"DISCONNECT") == 0){
	error = prep_disconnect(impl,data);
    } else if(strcmp(cmd_or_resp,"IDENTIFY") == 0){
	error = prep_identify(impl,data);
    } else if(strcmp(cmd_or_resp,"STARTTLS") == 0){
	error = prep_starttls(impl,data);
    } else if(strcmp(cmd_or_resp,"UPNEXPAND") == 0){
	error = prep_upnexpand(impl,data);
    } else if(strcmp(cmd_or_resp,"SENDDATA") == 0){
	error = prep_sendata(impl,data);
    }
    

}

    /* Read data until we get a end of data marker */



struct icalcstps_server_stubs {
  icalerrorenum (*abort)(icalcstps* cstp);
  icalerrorenum (*authenticate)(icalcstps* cstp, char* mechanism, 
                                    char* data);
  icalerrorenum (*calidexpand)(icalcstps* cstp, char* calid);
  icalerrorenum (*capability)(icalcstps* cstp);
  icalerrorenum (*cont)(icalcstps* cstp, unsigned int time);
  icalerrorenum (*identify)(icalcstps* cstp, char* id);
  icalerrorenum (*disconnect)(icalcstps* cstp);
  icalerrorenum (*sendata)(icalcstps* cstp, unsigned int time, 
                               icalcomponent *comp);
  icalerrorenum (*starttls)(icalcstps* cstp, char* command, 
                                char* data);
  icalerrorenum (*upnexpand)(icalcstps* cstp, char* upn);
  icalerrorenum (*unknown)(icalcstps* cstp, char* command, char* data);
};


/********************** Client (Sender) Interfaces **************************/

struct icalcstpc_impl {
	int timeout;
	icalparser *parser;
	enum cstp_command command;
	char* next_output;
	char* next_input;	
};

icalcstps* icalcstpc_new();

void* icalcstpc_free(icalcstpc* cstpc);

/* Get the next string to send to the server */
char* icalcstpc_next_output(icalcstpc* cstp)
{
}

/* process the next string to send to the server */ 
int icalcstpc_next_input(icalcstpc* cstp)
{
}

/* After icalcstpc_next_input returns a 0, there are responses
   ready. use these to get them */
icalcstpc_response icalcstpc_first_response(icalcstpc* cstp);
icalcstpc_response icalcstpc_next_response(icalcstpc* cstp);

int icalcstpc_set_timeout(icalcstpc* cstp, int sec);

icalerrorenum icalcstpc_abort(icalcstpc* cstp)
{
    struct icalcstpc_impl* impl = (struct icalcstpc_impl*)cstp;

    icalerror_check_arg_re(cstp!=0,"cstp",ICAL_BADARG_ERROR);

    impl->next_output = "ABORT";

}

icalerrorenum icalcstpc_authenticate(icalcstpc* cstp, char* mechanism, 
                                        char* data, char* f(char*))
{
}

icalerrorenum icalcstpc_capability(icalcstpc* cstp)
{
}

icalerrorenum icalcstpc_calidexpand(icalcstpc* cstp,char* calid)
{
}

icalerrorenum icalcstpc_continue(icalcstpc* cstp, unsigned int time)
{
}

icalerrorenum icalcstpc_disconnect(icalcstpc* cstp)
{
}

icalerrorenum icalcstpc_identify(icalcstpc* cstp, char* id)
{
}

icalerrorenum icalcstpc_starttls(icalcstpc* cstp, char* command, 
                                    char* data, char * f(char*))
{
}

icalerrorenum icalcstpc_upnexpand(icalcstpc* cstp,char* calid)
{
}

icalerrorenum icalcstpc_sendata(icalcstpc* cstp, unsigned int time,
                                   icalcomponent *comp)
{
}




