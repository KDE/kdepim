#ifndef _KPILOT_LOGFILE_H
#define _KPILOT_LOGFILE_H
/* logFile.h                          KPilot
**
** Copyright (C) 2001 by Dan Pilone
** Copyright (C) 2004 by Reinhold Kainhofer
**
** This file defines the log window widget, which logs
** sync-messages during a HotSync.
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

#include "loggerDCOP.h"
#include "qobject.h"

class LogFile : public QObject, public LoggerDCOP
{
Q_OBJECT

public:
	LogFile();
	~LogFile() { } ;

	/**
	* DCOP interface.
	*/
	virtual ASYNC logStartSync();
	virtual ASYNC logEndSync();
	virtual ASYNC logError(QString);
	virtual ASYNC logMessage(QString);
	virtual ASYNC logProgress(QString,int);

	void addMessage(const QString &);

private:
	QFile*fOutfile;
	bool fSyncing;
	QTextStream fLogStream;
} ;

#endif
