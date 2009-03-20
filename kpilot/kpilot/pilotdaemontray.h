#ifndef KPILOT_PILOTDAEMONTRAY_H
#define KPILOT_PILOTDAEMONTRAY_H
/* pilotdaemontray.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 by Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
**
** See the .cc file for an explanation of what this file is for.
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <ksystemtrayicon.h>

class KAboutData;

class PilotDaemonTray : public KSystemTrayIcon
{
	Q_OBJECT

public:
	explicit PilotDaemonTray( QWidget* parent = 0);

	~PilotDaemonTray();

	typedef enum { Normal, Busy, NotListening } IconShape ;

	void changeIcon( IconShape );
	void setAboutData( const KAboutData* aboutData );
	void selectDefaultSyncType();
	void startBlinking();
	void stopBlinking();

signals:
	void nextSyncChangedTo( int );
	void startConfigurationRequest();
	void startKPilotRequest();
	
protected:
	void enableRunKPilot( bool );
	void setupWidget();

protected slots:
	void slotBusyTimer();
	void slotHandleActionTrigger( QAction* );
	void slotHandleActivation( QSystemTrayIcon::ActivationReason reason );
	void slotHandleDefaultSyncToggle( bool );
	void slotShowBusy();
	void slotShowNormal();
	void slotShowNotListening();

private:
	class Private;
	Private* d;
};

#endif
