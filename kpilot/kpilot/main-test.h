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
	KPilotTestLink(const QString &);

public:
	static KPilotTestLink *getTestLink(const QString &p=QString::null);

private:
	static KPilotTestLink *fTestLink;

protected slots:
	void enumerateDatabases();
} ;

// $Log:$

#endif
