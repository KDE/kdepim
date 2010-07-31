#ifndef _KPILOT_PROBEDIALOG_H
#define _KPILOT_PROBEDIALOG_H
/* kpilotConfigWizard.h                 KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines kpilot's configuration wizard
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

#include "kdialogbase.h"

class KPilotDeviceLink;
class QLabel;
class QGroupBox;
class KProgress;
class QTimer;
class QGridLayout;
template <class T> class QValueList;
template<class Key, class T> class QMap;

typedef TQValueList<KPilotDeviceLink*> PilotLinkList;

typedef TQMap<TQString, KPilotDeviceLink*> PilotLinkMap;

class ProbeDialog : public KDialogBase
{
Q_OBJECT
public:
	ProbeDialog(TQWidget *p=0L,const char *n=0L);
	~ProbeDialog();

	bool detected() const { return mDetected; }
	TQString userName() const { return mUserName; }
	TQString device() const { return mDevice; }
	TQStringList dbs()  const { return mDBs; }


protected slots:
	void startDetection();
	void timeout();
	void connection(KPilotDeviceLink*lnk);
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
	TQLabel* fInfoText;
	TQGroupBox* fResultsGroup;
	TQLabel* fUserLabel;
	TQLabel* fDeviceLabel;
	TQLabel* fUser;
	TQLabel* fDevice;
	TQGroupBox* fStatusGroup;
	TQLabel* fStatus;
	KProgress* fProgress;

	TQTimer* fProcessEventsTimer;
	TQTimer* fTimeoutTimer;
	TQTimer* fProgressTimer;
	TQTimer* fRotateLinksTimer;
protected:
	TQGridLayout* fResultsGroupLayout;
	TQGridLayout* fStatusGroupLayout;

	TQStringList mDevicesToProbe[3];
	PilotLinkList mDeviceLinks[3];
	int mProbeDevicesIndex;
	KPilotDeviceLink *mActiveLink;

	bool mDetected;
	TQString mUserName;
	TQString mDevice;
	TQStringList mDBs;
} ;

#endif
