#ifndef _KPILOT_LOGWIDGET_H
#define _KPILOT_LOGWIDGET_H
/* logWidget.h                          KPilot
**
** Copyright (C) 2001 by Dan Pilone
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

#include "pilotComponent.h"
//Added by qt3to4:
#include <QLabel>

class QLabel;
class Q3TextEdit;
class KHBox;
class QProgressBar;

class LogWidget : public PilotComponent
{
Q_OBJECT

public:
	LogWidget(QWidget *);
	~LogWidget() { } ;

	// Pilot Component Methods:
	//
	bool showTime() const { return fShowTime; } ;
	void setShowTime(bool b) { fShowTime=b; } ;

	/**
	* D-Bus interface.
	*/
	virtual void logError(QString);
	virtual void logMessage(QString);
	virtual void logProgress(QString,int);
	virtual void logStartSync();
	virtual void logEndSync();

	// GUI customization hooks
	//
	//
	KHBox *buttonBox() const { return fButtonBox; } ;

public slots:
	void addMessage(const QString &);
	void addError(const QString &);
	void addProgress(const QString &,int);
	void syncDone();

private slots:
	void hideSplash();
	void clearLog();
	void saveLog();

private:
	bool saveFile(const QString &);

private:
	Q3TextEdit *fLog;
	bool fShowTime;
	QLabel *fSplash;
	QLabel *fLabel;
	QProgressBar *fProgress;
	KHBox *fButtonBox;
} ;

#endif
