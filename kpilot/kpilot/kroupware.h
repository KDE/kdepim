#ifndef _KPILOT_KROUPWARE_H
#define _KPILOT_KROUPWARE_H
/* kroupware.h			KPilot
** 
** Copyright still to be determined.
**
** This file defines the actions taken when KPilot
** is Kroupware-enabled. Basically it just does a
** little communication with the local Kroupware agent (KMail).
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "syncAction.h"

class QString;

class KroupwareSync : public SyncAction
{
public:
	// Creates a pre- or post-sync action
	// depending on @p pre (pre==true makes
	// it a pre-sync action, pre==false makes
	// it a post-sync action).
	//
	// Which things it syncs is based on
	// @p parts, which is a bitwise OR of
	// values from the KroupwareParts enum.
	//
	KroupwareSync(bool pre,int parts,KPilotDeviceLink *p);
	
	enum KroupwareParts 
	{
		Cal=1,
		Todo=2,
		Notes=4,
		Address=8
	} ;
	
protected:
	virtual bool exec();
	
	void preSync();   // Functions to collect all the actions
	void postSync();  //   together. Call {start,end}*().
	
protected:
	bool fPre;
	int fParts;
	
private:
	bool _syncWithKMail;

	void cleanupConfig();
	void start_syncCal_TodosWithKMail( bool cal, bool todos);
	void start_syncAddWithKMail();
	void start_syncNotesWithKMail();
	void end_syncCal_TodosWithKMail( bool cal, bool todos);
	void end_syncAddWithKMail();
	void end_syncNotesWithKMail();

	
public:
	/* Try to start KMail. Returns true on success. */
	static bool startKMail(QString *errormessage);
} ;

#endif

