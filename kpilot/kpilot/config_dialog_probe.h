#ifndef CONFIG_DIALOG_PROBE_H
#define CONFIG_DIALOG_PROBE_H
/* KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines a dialog that automatically probes for Palm devices.
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


#include <QLabel>
#include <QList>
#include <QString>
#include <QStringList>
#include <QGroupBox>

#include <kdialog.h>

class QLabel;
class QProgressBar;
class QTimer;

class KPilotDeviceLink;


class ProbeDialog : public KDialog
{
Q_OBJECT
public:
	typedef QList<KPilotDeviceLink*> PilotLinkList;

	explicit ProbeDialog(QWidget *p=0L,const char *n=0L);
	~ProbeDialog();

	bool detected() const { return fDetected; }
	QString userName() const { return fUserName; }
	QString device() const { return fDeviceName; }
	QStringList dbs()  const { return fDBs; }


protected slots:
	void startDetection();
	void timeout();
	void connection(KPilotLink*lnk);
	void retrieveDBList();
	void disconnectDevices();
	void processEvents();
	void progress();
	void detect();
	void detect(int i);
public slots:
	int exec();
	void slotUser1 () { startDetection(); }
protected:
	QGroupBox *fResultsGroup;
	QLabel *fStatusLabel;
	QLabel *fUserNameLabel;
	QLabel *fDeviceNameLabel;
	QProgressBar *fProgress;

	QTimer *fProcessEventsTimer;
	QTimer *fTimeoutTimer;
	QTimer *fProgressTimer;
	QTimer *fRotateLinksTimer;

	QStringList fDevicesToProbe[3];
	PilotLinkList fDeviceLinks[3];
	int fProbeDevicesIndex;
	KPilotDeviceLink *fActiveLink;

	bool fDetected;
	QString fUserName;
	QString fDeviceName;
	QStringList fDBs;
} ;

#endif
