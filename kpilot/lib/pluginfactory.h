#ifndef _KPILOT_PLUGINFACTORY_H
#define _KPILOT_PLUGINFACTORY_H
/* KPilot
**
** Copyright (C) 2005-2007 by Adriaan de Groot <groot@kde.org>
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
#include "options.h"

#include <QtGui/QWidget>

#include <kcomponentdata.h>
#include <klibloader.h>


/** @file Defines a template class for factories for KPilot's conduits. */

class KPilotLink;



/**
 * Template class that defines a conduit's factory. Instantiate it with a
 * configuration widget class and a SyncAction derived conduit action,
 * but preferably use DECLARE_KPILOT_PLUGIN below.
 */

template <class Widget, class Action> class ConduitFactory : public KLibFactory
{
public:
	ConduitFactory(QObject *parent = 0, const char * = 0) :
		KLibFactory(parent)
		{ /*fInstance(name);*/ } ;
	virtual ~ConduitFactory()
		{ /*delete fInstance;*/ } ;

protected:
	virtual QObject *createObject(
		QObject *parent = 0,
		const char *classname = "QObject",
		const QStringList &args = QStringList() )
	{
		if (qstrcmp(classname,"ConduitConfigBase")==0)
		{
			QWidget *w = dynamic_cast<QWidget *>(parent);
			if (w)
			{
				return new Widget(w);
			}
			else
			{
				WARNINGKPILOT <<"Could not cast parent to widget.";
				return 0L;
			}
		}

		if (qstrcmp(classname,"SyncAction")==0)
		{
			KPilotLink *d = 0L;
			if (parent) d = dynamic_cast<KPilotLink *>(parent);

			if (d || !parent)
			{
				if (!parent)
				{
					kDebug() <<": Using NULL device.";
				}
				return new Action(d,args);
			}
			else
			{
				WARNINGKPILOT <<"Could not cast parent to KPilotLink";
				return 0L;
			}
		}
		return 0L;
	}

	//KComponentData fInstance;
} ;

/**
 * A conduit has a name -- which must match the name of the library
 * that it lives in -- and two classes: a configure widget which derives
 * from ConduitConfigBase and a conduit action that derives from
 * ConduitAction. The boilerplate needed to handle the plugin
 * factory name and special symbols as well as the factory
 * is hidden in this macro.
 *
 * @param a The name of the conduit.
 * @param b The class name for the config widget.
 * @param c The class name for the conduit action.
 *
 * @note No quotes around the name.
 * @example DECLARE_KPILOT_PLUGIN(null, NullConfigWidget, ConduitNull)
 */
#define DECLARE_KPILOT_PLUGIN(a,b,c) \
	extern "C" { \
	KPILOT_EXPORT unsigned long version_##a = Pilot::PLUGIN_API; \
	KPILOT_EXPORT void *init_##a() \
	{ return new ConduitFactory<b,c>(0, #a); } }

#endif

