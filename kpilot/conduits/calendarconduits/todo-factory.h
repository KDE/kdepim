#ifndef _KPILOT_TODO_FACTORY_H
#define _KPILOT_TODO_FACTORY_H
/* todo-factory.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the todo-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "vcal-factorybase.h"

class KInstance;

class ToDoConduitFactory : public VCalConduitFactoryBase
{
    Q_OBJECT

public:
	ToDoConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~ToDoConduitFactory();
	static VCalConduitSettings*config();

protected:
	virtual QObject* createObject( QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static VCalConduitSettings*fConfig;
};

extern "C"
{

void *init_libtodoconduit();

}

#endif
