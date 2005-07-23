#ifndef _PYTHON_PYTHON_CONDUIT_H
#define _PYTHON_PYTHON_CONDUIT_H
/* python-conduit.h			KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file is part of the Python conduit, a conduit for KPilot that
** runs a Python interpreter in the context of  HotSync. The Python
** interpreter can do what it likes. This is probably most useful
** in the context of using jpilot-python conduits.
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

#include "plugin.h"

class PythonThread;

class PythonConduit : public ConduitAction
{
public:
	PythonConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~PythonConduit();

protected:
	virtual bool exec();           // From ConduitAction
	virtual bool event(QEvent *e); // From QObject

	PythonThread *fThread;           // A QThread subclass for the interpreter
};

#endif
