#ifndef _KPILOT_KPILOT_H
#define _KPILOT_KPILOT_H
/* kpilot.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is the main program in KPilot.
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

#include <QCloseEvent>

#include <KXmlGuiWindow>

#include <kpagewidgetmodel.h>

#include "pilotComponent.h"



class OrgKdeKpilotDaemonInterface;


class KPilotInstaller : public KXmlGuiWindow
{
Q_OBJECT

public:
	KPilotInstaller();
	~KPilotInstaller();

	typedef QList<PilotComponent *> ComponentList;

	enum DaemonMessages {
		None=0,
		StartOfHotSync=1,
		EndOfHotSync=2,
		DaemonQuit=4 } ;
	enum KPilotStatus {
		Startup=1,
		WaitingForDaemon=2,
		Normal=10,
		UIBusy=100,
		Error=101 } ;

	/**
	* This is the D-Bus interface from the daemon to KPilot.
	*/
	virtual void daemonStatus(int);
	virtual int kpilotStatus() const; ///< Returns KPilotStatus values
	virtual void raise();



public slots:
	/**
	* These are slots for the menu actions for each kind of
	* sync that can be requested.
	*/
	void slotRestoreRequested();
	void slotBackupRequested();
	void slotHotSyncRequested();
	void slotFullSyncRequested();
	void slotHHtoPCRequested();
	void slotPCtoHHRequested();

	virtual void configure();


protected slots:
	/**
	* Handle the functionality of kill-daemon-on-exit and
	* kill-daemon-if-started-by-my by killing it in those
	* cases.
	*/
	void killDaemonIfNeeded();
	void startDaemonIfNeeded();

	void quit();

	/**
	 * Get the daemon to reset the link. This uses reloadSettings()
	 * to achieve this result - the daemon calls reset() in there.
	 */
	void slotResetLink();

	void componentUpdate();
	void componentChanged(KPageWidgetItem*,KPageWidgetItem *);

protected:
	void readConfig();

	QWidget *initComponents( QWidget *parent, QList<PilotComponent *> &l );


	/**
	* Run all the internal conduits' presync functions.
	*/
	bool componentPreSync();
	void componentPostSync();

	void setupWidget();
	void setupSync(int kind,const QString& msg);


	/**
	* Provide access to the daemon's D-Bus interface
	* through an object of the stub class.
	*/
	OrgKdeKpilotDaemonInterface &getDaemon()
	{
		return *fDaemonInterface;
	}

private:
	OrgKdeKpilotDaemonInterface *fDaemonInterface;

	void log(QString s);

	/**
	* This is the private-d-pointer, KPilot style. Not everything
	* has moved there yet.
	*/
	class KPilotPrivate;
	KPilotPrivate *fP;

};




#endif
