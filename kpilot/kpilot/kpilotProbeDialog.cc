/* conduitConfigDialog.cc                KPilot
**
** Copyright (C) 2004 by Dan Pilone
** Written 2004 by Reinhold Kainhofer
**
** This file defines a .ui-based configuration dialog for conduits.
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtimer.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfigskeleton.h>
#include <kapplication.h>
#include <kprogress.h>

#include "kpilotConfig.h"
#include "pilotUser.h"
#include "pilotSysInfo.h"
#include "options.h"
#include "kpilotlink.h"

#include "kpilotProbeDialog.moc"
#ifndef __PILOTDAEMONDCOP_STUB__
#include "pilotDaemonDCOP_stub.h"
#endif



ProbeDialog::ProbeDialog(QWidget *parent, const char *n) :
	KDialogBase(parent, n, true, i18n("Autodetecting Your Handheld"), KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::User1, KDialogBase::Cancel, true, i18n("Restart detection...")),
	mDetected(false), mUserName(""), mDevice(""), mUID(0)
{
	QVBox *mainWidget = makeVBoxMainWidget();

	fInfoText = new QLabel( i18n( "KPilot is now trying to automatically detect the device of your handheld. Please press the hotsync button if you have not done so already." ), mainWidget, "fInfoText" );
	fInfoText->setAlignment( QLabel::WordBreak );

	fStatusGroup = new QGroupBox( i18n("Status"), mainWidget, "fStatusGroup" );
	fStatusGroup->setColumnLayout(0, Qt::Vertical );
	fStatusGroupLayout = new QGridLayout( fStatusGroup->layout() );

	fStatus = new QLabel( i18n("Autodetection not yet started..."), fStatusGroup, "fStatus" );
	fStatus->setAlignment( QLabel::WordBreak );
	fStatusGroupLayout->addWidget( fStatus, 0, 0 );
	
	fProgress = new KProgress( 60, fStatusGroup, "fProgress" );
	fStatusGroupLayout->addWidget( fProgress, 1, 0 );



	fResultsGroup = new QGroupBox( i18n( "Detected Values" ), mainWidget, "fResultsGroup" );
	fResultsGroup->setEnabled( FALSE );
	fResultsGroup->setColumnLayout(0, Qt::Vertical );
	fResultsGroupLayout = new QGridLayout( fResultsGroup->layout() );
	fResultsGroupLayout->setAlignment( Qt::AlignTop );

	fUserLabel = new QLabel( i18n( "Handheld user:" ), fResultsGroup, "fUserLabel" );
	fUserLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)5, 0, 0, fUserLabel->sizePolicy().hasHeightForWidth() ) );
	fResultsGroupLayout->addWidget( fUserLabel, 0, 0 );

	fDeviceLabel = new QLabel( i18n( "Device:" ), fResultsGroup, "fDeviceLabel" );
	fResultsGroupLayout->addWidget( fDeviceLabel, 1, 0 );

	fUser = new QLabel( i18n("[Not yet known]"), fResultsGroup, "fUser" );
	fResultsGroupLayout->addWidget( fUser, 0, 1 );

	fDevice = new QLabel( i18n("[Not yet known]"), fResultsGroup, "fDevice" );
	fResultsGroupLayout->addWidget( fDevice, 1, 1 );


	resize( QSize(459, 298).expandedTo(minimumSizeHint()) );
	clearWState( WState_Polished );
	enableButtonOK(false);

	mDevicesToProbe //<< "/dev/pilot"
	                <<"/dev/ttyS0"<<"/dev/ttyS1"<<"/dev/ttyS2"<<"/dev/ttyS3"
//	                <<"/dev/tts/0"<<"/dev/tts/1"<<"/dev/tts/2"<<"/dev/tts/3"
	                <<"/dev/ttyUSB0"<<"/dev/ttyUSB1"<<"/dev/ttyUSB2"<<"/dev/ttyUSB3"
//	                <<"/dev/usb/tts/0"<<"/dev/usb/tts/1"<<"/dev/usb/tts/2"<<"/dev/usb/tts/3"
	                <<"/dev/cuaa0"<<"/dev/cuaa1"<<"/dev/cuaa2"<<"/dev/cuaa3"
	                <<"/dev/ucom0"<<"/dev/ucom1"<<"/dev/ucom2"<<"/dev/ucom3";
;
	
	fProcessEventsTimer = new QTimer( this );
	fTimeoutTimer = new QTimer( this );
	fProgressTimer = new QTimer( this );
	connect( fProcessEventsTimer, SIGNAL(timeout()), this, SLOT(processEvents()) );
	connect( fTimeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
	connect( fProgressTimer, SIGNAL(timeout()), this, SLOT( progress()) );
	connect( this, SIGNAL(finished()), this, SLOT(disconnectDevices()) );
}

ProbeDialog::~ProbeDialog()
{
}

void ProbeDialog::processEvents() {
FUNCTIONSETUP;
//kdDebug()<<"processEvents"<<endl;
//QTimer::singleShot(500, this, SLOT(processEvents()));
	KApplication::kApplication()->processEvents();
}

void ProbeDialog::progress() {
	fProgress->advance(1);
}

int ProbeDialog::exec() {
	mDetected = false;
	mUserName = "";
	mDevice = "";
	mUID = 0;
	QTimer::singleShot( 0, this, SLOT( startDetection() ) );
	return KDialogBase::exec();
}

// Devices to probe:
// Linux: /dev/pilot (symlink), /dev/ttyS* (serial + irda), /dev/tts/[012345...] (with devfs),
//        /dev/ttyUSB*, /dev/usb/tts/[012345...]
// *BSD: /dev/pilot, /dev/cuaa[01]   (serial), /dev/ucom* (usb)

void ProbeDialog::startDetection() {
	disconnectDevices();
	fProgress->setProgress(0);
	fStatus->setText( i18n("Starting detection...") );
	QTimer::singleShot(0, this, SLOT(processEvents()) );
	processEvents();
	PilotDaemonDCOP_stub *daemonStub = new PilotDaemonDCOP_stub("kpilotDaemon", "KPilotDaemonIface");
	if (daemonStub) {
		daemonStub->stopListening();
	}
	KPILOT_DELETE(daemonStub);
	processEvents();
	if (!fTimeoutTimer->start( 30000, true ) ) kdDebug()<<"Could not start fTimeoutTimer"<<endl;
	if (!fProcessEventsTimer->start( 100, false ) ) kdDebug()<<"Could not start fProcessEventsTimer"<<endl;
	if (!fProgressTimer->start( 500, false) ) kdDebug()<<"Could not start Progress timer"<<endl;
	
	QStringList::iterator end(mDevicesToProbe.end());
	KPilotDeviceLink*link;
	for (QStringList::iterator it=mDevicesToProbe.begin(); it!=end; ++it) {
		link = new KPilotDeviceLink();
kdDebug()<<"new kpilotDeviceLink for "<<(*it)<<endl;
		link->reset( KPilotDeviceLink::OldStyleUSB, *it);
kdDebug()<<"after resetting kpilotDeviceLink for "<<(*it)<<endl;
		mDeviceLinkMap[*it] = link;
		mDeviceLinks.append( link );
		connect( link, SIGNAL(deviceReady(KPilotDeviceLink*)), this, SLOT(connection(KPilotDeviceLink*)) );
	}
	fStatus->setText( i18n("Waiting for handheld to connect...") );
kdDebug()<<"end of startDetection"<<endl;
QTimer::singleShot(0, this, SLOT(processEvents()));
}

void ProbeDialog::timeout() {
	disconnectDevices();
	if (!mDetected) fStatus->setText( i18n("Timeout reached, could not detect a handheld.") );
}

void ProbeDialog::connection( KPilotDeviceLink*lnk) {
	if (!lnk) return;
	KPilotUser*usr( lnk->getPilotUser() );
//	KPilotSysInfo*sysInfo( lnk->getSysInfo() );
	
	mUserName = usr->getUserName();
	mUID = usr->getUserID();
	mDevice = lnk->pilotPath();
	lnk->endOfSync();

	QTimer::singleShot(0, this, SLOT(disconnectDevices()));
	fStatus->setText( i18n("Found a connected device on %1").arg(mDevice) );
	fUser->setText( mUserName );
	fDevice->setText( mDevice );
	mDetected = true;

	fResultsGroup->setEnabled( true );
	enableButtonOK(true);
}

void ProbeDialog::disconnectDevices() {
	if (!mDetected) fStatus->setText( i18n("Disconnected from all devices") );
	fProcessEventsTimer->stop( );
	fTimeoutTimer->stop();
	fProgressTimer->stop();
	fProgress->setProgress(fProgress->maxValue());
	if (!mDeviceLinks.isEmpty()) {
		PilotLinkList::iterator end(mDeviceLinks.end());
		for (PilotLinkList::iterator it=mDeviceLinks.begin(); it!=end; ++it) {
			KPILOT_DELETE(*it);
		}
		mDeviceLinks.clear();
		mDeviceLinkMap.clear();
	}
	

	PilotDaemonDCOP_stub *daemonStub = new PilotDaemonDCOP_stub("kpilotDaemon", "KPilotDaemonIface");
	if (daemonStub) {
		daemonStub->startListening();
	}
	KPILOT_DELETE(daemonStub);
}

