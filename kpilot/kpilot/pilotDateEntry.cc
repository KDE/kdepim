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
  fAppointmentInfo.description = (char*)malloc(strlen(desc) + 1);
  strcpy(fAppointmentInfo.description, desc);
}

void  
PilotDateEntry::setNote(const char* note)
{
  if(fAppointmentInfo.note)
    free(fAppointmentInfo.note);
  fAppointmentInfo.note = (char*)malloc(strlen(note) + 1);
  strcpy(fAppointmentInfo.note, note);
}

