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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/
#ifndef _KPILOT_LOGWIDGET_H
#define _KPILOT_LOGWIDGET_H

#include "pilotComponent.h"

class QTextView;

class LogWidget : public PilotComponent
{
Q_OBJECT

public:
	LogWidget(QWidget *);
	~LogWidget() { } ;

	// Pilot Component Methods:
	//
	void initialize();

	bool showTime() const { return fShowTime; } ;
	void setShowTime(bool b) { fShowTime=b; } ;

public slots:
	void addMessage(const QString &);
	void addError(const QString &);
	void syncDone();

private:
	QTextView *fLog;
	bool fShowTime;
} ;

#endif
