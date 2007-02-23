/* -*- Mode: C -*- */
/*======================================================================
 FILE: icalclassify.h
 CREATOR: eric 21 Aug 2000



 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

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

#include "ical.h"

icalproperty_xlicclass icalclassify(icalcomponent* c,icalcomponent* match, 
			      const char* user);

#endif /* ICALCLASSIFY_H*/


				    


