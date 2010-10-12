#ifndef PILOTDAEMONDCOP_H
#define PILOTDAEMONDCOP_H
/* pilotDaemonDCOP.h			KPilotDaemon
**
** Copyright (C) 2000 by Adriaan de Groot
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the DCOP interface for
** the KPilotDaemon. The daemon has *two* interfaces:
** one belonging with KUniqueApplication and this one.
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


#include <dcopobject.h>
#include <qdatetime.h>
#include <qstringlist.h>

class PilotDaemonDCOP : virtual public DCOPObject
{
	K_DCOP
public:
k_dcop:
	/**
	* Start a HotSync. What kind of HotSync is determined
	* by the int parameter (use the enum in kpilot.kcfg, or
	* better yet, use requestSyncType and pass the name).
	* Using a value of 0 (zero, which isn't a legal mode for
	* sync actions) uses the configuration file default.
	*/
	virtual ASYNC requestSync(int) = 0;
	/**
	* Request a particular kind of sync next; pass in the name
	* of a sync type instead.
	*/
	virtual ASYNC requestSyncType(QString) = 0;
	/** Shortcut for using requestSync(1) */
	virtual ASYNC requestRegularSyncNext() = 0;
	/** Query what type is set most recently. */
	virtual int nextSyncType() const = 0;
	/** Set the mix-ins (see SyncAction::SyncMode for details). */
	virtual ASYNC requestSyncOptions(bool test, bool local) = 0;

	/**
	* Functions for the KPilot UI, indicating what the daemon
	* should do.
	*/
	virtual ASYNC quitNow() = 0;
	virtual ASYNC reloadSettings() = 0; // Indicate changed config file.
	virtual ASYNC setTempDevice(QString d) =0; // use this device
	virtual void stopListening() = 0;
	virtual void startListening() = 0;
	virtual bool isListening() =0 ;

	/**
	* Functions requesting the status of the daemon.
	*/
	virtual QString statusString() = 0;
	virtual QString shortStatusString() = 0;

	/**
	* Functions reporting same status data, e.g. for the kontact plugin.
	*/
	virtual QDateTime lastSyncDate() = 0;
	virtual QStringList configuredConduitList() = 0;
	virtual QString logFileName() = 0;
	virtual QString userName() = 0;
	virtual QString pilotDevice() = 0;
	virtual bool killDaemonOnExit() = 0;

	/**
	* Some other useful functionality
	*/
	virtual void addInstallFiles(const QStringList &) = 0;


k_dcop_signals:
	void kpilotDaemonStatusChanged();
	// used to push status information
	void kpilotDaemonStatusDetails( QDateTime, QString, QStringList, QString, QString, QString, bool );
} ;

#endif
