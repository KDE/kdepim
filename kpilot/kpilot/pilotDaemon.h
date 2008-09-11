#ifndef KPILOT_PILOTDAEMON_H
#define KPILOT_PILOTDAEMON_H
/* pilotDaemon.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

#include <QtGui/QCloseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtGui/QDragEnterEvent>

#include "kpilotlink.h"
#include "syncAction.h"

class KPilotDeviceLink;
class PilotDaemonTray;

class PilotDaemon : public QObject
{
Q_OBJECT

public:
	PilotDaemon();
	
	~PilotDaemon();

	enum DaemonStatus
	{
		HOTSYNC_START,    // Hotsync is running
		HOTSYNC_END,      // Hotsync is cleaning up
		FILE_INSTALL_REQ, // A file is being saved for installation
		ERROR,            // Some error has occurred
		READY,            // Connected to device and ready for Sync
		INIT,
		NOT_LISTENING
	};

	QStringList configuredConduitList();
	int getPilotSpeed();
	bool isListening() const;
	bool killDaemonOnExit() const;
	QDateTime lastSyncDate();
	QString logFileName() const;
	int nextSyncType() const;
	QString pilotDevice() const;
	void quitNow();
	void reloadSettings();
	void requestRegularSyncNext();
	void requestSyncType( const QString& );
	void requestSyncOptions( bool, bool );
	void setTempDevice( const QString& d );
	QString shortStatusString() const;
	/**
	 * Display the daemon's system tray icon
	 * (if there is one, depending on the DockDaemon
	 * setting in the config file)
	 */
	void showTray();
	DaemonStatus status() const;
	void startListening();
	QString statusString() const;
	void stopListening();
	QString userName() const;

public slots:
	void endHotSync();
	void logError( const QString& );
	void logMessage( const QString& );
	void logProgress( const QString&, int );
	void requestSync( int );
	void startHotSync( KPilotLink* lnk );
	void startKPilot();
	void runConfig();
	
private: // Functions
	/**
	* Check whether we should do a backup.  This is based on the
	* KPilotSettings::backupFrequency and uses
	* SyncAction::BackupFrequency.  This will be expanded, hopefully,
	* to provide backup scheduling at some point.
	*/
	bool shouldBackup();

	bool setupPilotLink();
	
	/**
	* Set or change the tooltip displayed by the tray icon.
	*/
	void updateTrayStatus( const QString &s = QString() );

private: // Members
	class Private;
	Private* d;
};


#endif
