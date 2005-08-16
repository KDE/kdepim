#ifndef _KPILOT_PLUGIN_FACTORY_H
#define _KPILOT_PLUGIN_FACTORY_H
/* plugin_factory.h                        KPilot
**
** Copyright (C) 2005 by Adriaan de Groot
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

/** @file Defines a template class for factories for KPilot's conduits. */

#include <qwidget.h>
#include <klibloader.h>
#include <kdebug.h>

class KPilotDeviceLink;

/** Template class that defines a conduit's factory. */

template <class Widget, class Action> class ConduitFactory : public KLibFactory
{
public:
	ConduitFactory(QObject *parent = 0, const char *name = 0) :
		KLibFactory(parent,name)
		{ fInstance = new KInstance(name); } ;
	virtual ~ConduitFactory()
		{ delete fInstance; } ;

protected:
	virtual QObject *createObject(
		QObject* parent = 0,
		const char* name = 0,
		const char* classname = "QObject",
		const QStringList &args = QStringList() )
	{
		if (qstrcmp(classname,"ConduitConfigBase")==0)
		{
			QWidget *w = dynamic_cast<QWidget *>(parent);
			if (w) return new Widget(w,name);
			else
			{
				kdError() << k_funcinfo
					<< ": Couldn't cast parent to widget."
					<< endl;
				return 0L;
			}
		}

		if (qstrcmp(classname,"SyncAction")==0)
		{
			KPilotDeviceLink *d = 0L;
			if (parent) d = dynamic_cast<KPilotDeviceLink *>(parent);

			if (d || !parent)
			{
				if (!parent)
				{
					kdDebug() << k_funcinfo << ": Using NULL device." << endl;
				}
				return new Action(d,name,args);
			}
			else
			{
				kdError() << k_funcinfo
					<< ": Couldn't cast parent to KPilotDeviceLink"
					<< endl;
				return 0L;
			}
		}
		return 0L;
	}

	KInstance *fInstance;
} ;

#endif

