/* logFile.cc                         KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2004 by Reinhold Kainhofer
**
** This file defines the log file class, which logs
** sync-messages during a HotSync to a file.
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
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
static const char *logf_id =
	"$Id$";

#include "options.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>

#include <pi-version.h>

#ifndef PILOT_LINK_PATCH
#define PILOT_LINK_PATCH "unknown"
#endif
#include "logFile.h"
#include "kpilotConfig.h"

#include "logFile.moc"


LogFile::LogFile() : DCOPObject("LogIface"), QObject(), fOutfile(0L), fSyncing(false)
{
	FUNCTIONSETUP;
	(void) logf_id;
}


/* DCOP */ ASYNC LogFile::logStartSync()
{
	FUNCTIONSETUP;
	// If a sync is already running (something went wrong then!), close that old log
	if (fSyncing) logEndSync();
	
	fOutfile = new QFile(KPilotSettings::logFileName());
	
	if (!fOutfile || !fOutfile->open(IO_WriteOnly)) 
	{
		DEBUGKPILOT<<"Unable to open log file "<<KPilotSettings::logFileName()<<endl;
		kdWarning()<<"Unable to open log file "<<KPilotSettings::logFileName()<<endl;
		KPILOT_DELETE( fOutfile );
		fSyncing = false;
		return;
	}
	
	fSyncing = true;
	fLogStream.setDevice(fOutfile);

	fLogStream<<(CSL1("KPilot HotSync log, %1").arg(QDateTime::currentDateTime().toString()))<<endl<<endl<<endl;
	fLogStream<<(CSL1("Version: KPilot %1").arg(QString::fromLatin1(KPILOT_VERSION)))<<endl;
	fLogStream<<(CSL1("Version: pilot-link %1.%2.%3%4" )
		.arg(PILOT_LINK_VERSION).arg(PILOT_LINK_MAJOR).arg(PILOT_LINK_MINOR)
#ifdef PILOT_LINK_PATCH
		.arg(QString::fromLatin1(PILOT_LINK_PATCH))
#else
		.arg(QString())
#endif
		)<<endl;
#ifdef KDE_VERSION_STRING
	fLogStream<<(CSL1("Version: KDE %1" ).arg(QString::fromLatin1(KDE_VERSION_STRING)) )<<endl;
#endif
#ifdef QT_VERSION_STR
	fLogStream<<(CSL1("Version: Qt %1" ).arg(QString::fromLatin1(QT_VERSION_STR)) )<<endl;
#endif
	fLogStream<<endl<<endl;
		
}

/* DCOP */ ASYNC LogFile::logEndSync()
{
	if (fSyncing) 
	{
		logMessage(i18n("HotSync finished."));
		fLogStream.unsetDevice();
		if (fOutfile) fOutfile->close();
		KPILOT_DELETE(fOutfile)
		fSyncing=false;
	}
}

/* DCOP */ ASYNC LogFile::logMessage(QString s)
{
	addMessage(s);
}

/* DCOP */ ASYNC LogFile::logError(QString s)
{
	addMessage(s);
}

/* DCOP */ ASYNC LogFile::logProgress(QString, int)
{
}


void LogFile::addMessage(const QString & s)
{
	FUNCTIONSETUP;
	if ( fSyncing && !s.isEmpty() ) 
	{
		fLogStream<<QTime::currentTime().toString()<<"  "<<s<<endl;
	}
}

