/* systray.h                    KitchenSync
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _SYSTRAY_H
#define _SYSTRAY_H

#include <qpixmap.h>

#include <ksystemtray.h>

#include "logWindow.h"

class Config;
class ConfigDialog;
class SyncManager;

class SystrayWindow : public KSystemTray
{
Q_OBJECT

public:
	SystrayWindow(Config *,SyncManager *);

	void showTray();

	typedef enum { Idle, Busy } Icon;
	void setIcon(Icon);

public slots:
	void startSync(const QString &s);
	void endSync(const QString &s);

	void toggleStatusWindow();
	void statusWindowHidden();
	void statusWindowRevealed();

	void showConfigDialog();
	void configChanged();
	void configClosed();

signals:
	void configChanged(Config *);

protected:
	virtual void closeEvent(QCloseEvent *);

	void createLogWindow();
	void addDaemons();

private:
	QPixmap fIdleIcon, fBusyIcon;
	int fToggleStatusWindowId;
	Config *fConfig;

	SyncManager *fManager;
	LogWindow *fLog;
	ConfigDialog *fConfigDialog;
} ;

#endif

// $Log:$
