/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Preston Brown
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __KPILOT_TODO_ENTRY_H
#define __KPILOT_TODO_ENTRY_H

#include <qbitarray.h>
#include <time.h>
#include <string.h>

#include <pi-macros.h>
#include <pi-todo.h>

#include "pilotAppCategory.h"
#include "pilotRecord.h"


class PilotTodoEntry : public PilotAppCategory
{
public:
  PilotTodoEntry(void) : PilotAppCategory() 
    { memset(&fTodoInfo, 0, sizeof(struct ToDo)); }
  PilotTodoEntry(PilotRecord* rec);
  ~PilotTodoEntry() { free_ToDo(&fTodoInfo); }
  
  PilotRecord* pack() { return PilotAppCategory::pack(); }
  
  struct tm getDueDate() const { return fTodoInfo.due; }
  void setDueDate(struct tm& d) { fTodoInfo.due = d; }
  
  int getIndefinite() const { return fTodoInfo.indefinite; }
  void setIndefinite(int i) { fTodoInfo.indefinite = i; }

  int getPriority() const { return fTodoInfo.priority; }
  void setPriority(int p) { fTodoInfo.priority = p; }

  int getComplete() const { return fTodoInfo.complete; }
  void setComplete(int c) { fTodoInfo.complete = c; }

  void  setDescription(const char* desc);
  char* getDescription() { return fTodoInfo.description; }

  void  setNote(const char* note);
  char* getNote() { return fTodoInfo.note; }
  
protected:
  void *pack(void *, int *);
  void unpack(const void *, int = 0) { }
  
private:
  struct ToDo fTodoInfo;
};



#endif
