/* KPilot
**
** Copyright (C) 2004 by Adriaan de Groot
**
** This file is part of the Python conduit, a conduit for KPilot that
** is intended to showcase how to use python code inside a conduit.
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

#include "options.h"

#include <Python.h>

#include "python-conduit.h"  // The Conduit action
#include "pythonconduit.h"   // The settings class

#include <qthread.h>
#include <qapplication.h>

extern "C"
{
long version_conduit_python = KPILOT_PLUGIN_API;
const char *id_conduit_python =
	"$Id$";
}


class PythonThread : public QThread
{
public:
	PythonThread(QObject *parent) : fParent(parent) { } ;
	virtual void run();

	QString result() const { return fResult; } ;

protected:
	QObject *fParent;
	QString fResult;
} ;


PythonConduit::PythonConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	ConduitAction(d,n,l)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_python << endl;
#endif
	fConduitName=i18n("Python");

	(void) id_conduit_python;
}

PythonConduit::~PythonConduit()
{
	FUNCTIONSETUP;
}

/* virtual */ bool PythonConduit::exec()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": In exec() @" << (unsigned long) this << endl;
#endif

	fThread = new PythonThread(this) ;
	fThread->start();
	startTickle();
	return true;
}

/* virtual */ bool PythonConduit::event(QEvent *e)
{
	FUNCTIONSETUP;

	if (e->type() == QEvent::User)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": Python thread done." << endl;
#endif
		QString r;
		addSyncLogEntry(i18n("Python returned %1.").arg(fThread->result()));
		stopTickle();
		delayDone();
		return true;
	}
	else return ConduitAction::event(e);
}


void PythonThread::run()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread starting." << endl;
#endif

	PyObject *pName,*pArgs,*pValue;
	int i;

	Py_Initialize();
	pName = PyString_FromString("KPilot");
// TODO: This doesn't compile with my python2.3-dev (debian sid)
//	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Thread done with " << endl;
#endif


	QApplication::postEvent(fParent,new QEvent(QEvent::User));
}

