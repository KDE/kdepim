/* pilotDateEntry.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a C++ wrapper for the Pilot's datebook structures.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#include <stdlib.h>
#include "pilotDateEntry.h"

PilotDateEntry::PilotDateEntry(PilotRecord* rec)
  : PilotAppCategory(rec)
    {
    unpack_Appointment(&fAppointmentInfo, (unsigned char*)rec->getData(), rec->getLen());
    }

void*
PilotDateEntry::pack(void *buf, int *len)
    {
    int i;
    i = pack_Appointment(&fAppointmentInfo, (unsigned char*)buf, *len);
    *len = i;
    return buf;
    }

void  
PilotDateEntry::setDescription(const char* desc)
{
  if(fAppointmentInfo.description)
    free(fAppointmentInfo.description);
  if (desc)
    {
      fAppointmentInfo.description = (char*)malloc(strlen(desc) + 1);
      strcpy(fAppointmentInfo.description, desc);
    }
  else
    fAppointmentInfo.description = 0L;
}

void  
PilotDateEntry::setNote(const char* note)
{
  if(fAppointmentInfo.note)
    free(fAppointmentInfo.note);
  if (note)
    {
      fAppointmentInfo.note = (char*)malloc(strlen(note) + 1);
      strcpy(fAppointmentInfo.note, note);
    }
  else
    fAppointmentInfo.note = 0L;
}



// $Log:$
