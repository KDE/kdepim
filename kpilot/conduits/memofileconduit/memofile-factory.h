#ifndef _KPILOT_MEMOFILE_FACTORY_H
#define _KPILOT_MEMOFILE_FACTORY_H
/* memofile-factory.h                       KPilot
**
** Copyright (C) 2004-2004 by Jason 'vanRijn' Kasper
**
** This file defines the factory for the Memofile-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klibloader.h>

#include "plugin.h"

class MemofileWidget;
class KInstance;
class KAboutData;

class MemofileConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	MemofileConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~MemofileConduitFactory();

	static KAboutData *about() { return fAbout; } ;

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
} ;

extern "C"
{

void *init_libmemofileconduit();

}

#endif
