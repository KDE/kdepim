#include <stdlib.h>
#include "pilotTodoEntry.h"

PilotTodoEntry::PilotTodoEntry(PilotRecord* rec)
  : PilotAppCategory(rec)
{
  unpack_ToDo(&fTodoInfo, (unsigned char*)rec->getData(), rec->getLen());
}

void* PilotTodoEntry::pack(void *buf, int *len)
{
  int i;
  i = pack_ToDo(&fTodoInfo, (unsigned char*)buf, *len);
  *len = i;
  return buf;
}

void PilotTodoEntry::setDescription(const char* desc)
{
  if(fTodoInfo.description)
    free(fTodoInfo.description);
  fTodoInfo.description = (char*)malloc(strlen(desc) + 1);
  strcpy(fTodoInfo.description, desc);
}

void PilotTodoEntry::setNote(const char* note)
{
  if(fTodoInfo.note)
    free(fTodoInfo.note);
  fTodoInfo.note = (char*)malloc(strlen(note) + 1);
  strcpy(fTodoInfo.note, note);
}

