/* todo-conduit.h			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
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
#ifndef _TODOCONDUIT_H
#define _TODOCONDUIT_H

#include <config.h>

#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#include "../vcalconduit/vcalBase.h"

class QWidget;
class PilotRecord;
class VObject;


class TodoConduit : public VCalBaseConduit
{
public:
  TodoConduit(eConduitMode mode);
  virtual ~TodoConduit();
  
  virtual void doSync();
  virtual void doBackup();
  virtual QWidget* aboutAndSetup();

  virtual const char* dbInfo() { return "TodoDB,ToDoDB"; }
  
	static const char *version();


protected:
  void doLocalSync();
  //PilotRecord *findEntryInDB(unsigned int id);
//  VObject *findEntryInCalendar(unsigned int id);
//  void deleteVObject(PilotRecord *rec);
  void updateVObject(PilotRecord *rec);
//  void saveTodo();
//  QString TmToISO(struct tm tm);
//  struct tm ISOToTm(const QString &tStr);
//  int numFromDay(const QString &day);
//  int timeZone;
//  VObject *fCalendar;

private:
	/**
	* Get the calendar file from whereever. This
	* has moved to a separate function because
	* any (translated) error messages it produces
	* require a KApplication object, which is only
	* created when exec() is called.
	*/
//	void getCalendar();

//	QString calName;
};

#endif


// $Log$
// Revision 1.9  2001/03/04 13:46:49  adridg
// struct tm woes
//
// Revision 1.8  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
