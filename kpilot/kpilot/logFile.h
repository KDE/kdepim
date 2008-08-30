#ifndef KPILOT_LOGFILE_H
#define KPILOT_LOGFILE_H
/* logFile.h                          KPilot
**
** Copyright (C) 2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include <QtCore/QTextStream>

class QFile;

class LogFile : public QObject
{
Q_OBJECT

public:
	LogFile();
	~LogFile() { } ;

	/**
	* DBUS interface.
	*/
	virtual void logStartSync();
	virtual void logEndSync();
	virtual void logError(const QString&);
	virtual void logMessage(const QString&);
	virtual void logProgress(const QString&,int);

	void addMessage(const QString &);

private:
	QFile*fOutfile;
	bool fSyncing;
	QTextStream fLogStream;
} ;

#endif
