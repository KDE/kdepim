#ifndef _KPILOT_PROBEDIALOG_H
#define _KPILOT_PROBEDIALOG_H
/* kpilotConfigWizard.h                 KPilot
**
** Copyright (C) 2004 by Dan Pilone
** Written 2004 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "kdialogbase.h"
//#include <qmap.h>
//#include <qvaluelist.h>

class KPilotDeviceLink;
class QLabel;
class QGroupBox;
class KProgress;
class QTimer;
class QGridLayout;
template <class T> class QValueList;
template<class Key, class T> class QMap;

typedef QValueList<KPilotDeviceLink*> PilotLinkList;

typedef QMap<QString, KPilotDeviceLink*> PilotLinkMap;

class ProbeDialog : public KDialogBase
{
Q_OBJECT
public:
	ProbeDialog(QWidget *p=0L,const char *n=0L);
	~ProbeDialog();
	
	bool detected() { return mDetected; }
	QString userName() { return mUserName; }
	QString device() { return mDevice; }
	int userID() { return mUID; }
	

protected slots:
	void startDetection();
	void timeout();
	void connection(KPilotDeviceLink*lnk);
	void disconnectDevices();
	void processEvents();
	void progress();
public slots:
	int exec();
	void slotUser1 () { startDetection(); }
protected:
	QLabel* fInfoText;
	QGroupBox* fResultsGroup;
	QLabel* fUserLabel;
	QLabel* fDeviceLabel;
	QLabel* fUser;
	QLabel* fDevice;
	QGroupBox* fStatusGroup;
	QLabel* fStatus;
	KProgress* fProgress;

	QTimer* fProcessEventsTimer;
	QTimer* fTimeoutTimer;
	QTimer* fProgressTimer;

protected:
	QGridLayout* fResultsGroupLayout;
	QGridLayout* fStatusGroupLayout;
	
	QStringList mDevicesToProbe;
	PilotLinkList mDeviceLinks;
	PilotLinkMap mDeviceLinkMap;
	
	bool mDetected;
	QString mUserName;
	QString mDevice;
	int mUID;
} ;

#endif
