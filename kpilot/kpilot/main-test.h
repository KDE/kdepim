#ifndef _KPILOT_MAIN_TEST_H
#define _KPILOT_MAIN_TEST_H
/* main-test.h                          KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This defines the class KPilotTestLink, which just exercises
** some of the code in the KPilotDeviceLink class.
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
** Bug reports and questions can be sent to groot@kde.org
*/

#include <qstring.h>
#include "kpilotlink.h"

class KPilotTestLink : public KPilotDeviceLink
{
Q_OBJECT

protected:
	KPilotTestLink(DeviceType t,const QString &);

public:
	static KPilotTestLink *getTestLink(DeviceType=None,
		const QString &p=QString::null);

private:
	static KPilotTestLink *fTestLink;

protected slots:
	void enumerateDatabases();
} ;

// $Log$
// Revision 1.1  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//

#endif
