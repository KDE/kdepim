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
#include <qptrlist.h>
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

/*
We can't connect to /dev/ttyUSB0 and /dev/ttyUSB1 at the same time, because that
will lock up kpilot completely. In particular, it gets a connection on /dev/ttyUSB0,
which it processes, and while processing, a connection on USB1 is also detected.
However, when kpilot gets 'round to process it, the link is already closed, and
pi_connect hangs forever.

Now, I split up the list of devices to probe into three list, one holding /dev/pilot,
the second holding all /dev/xxx0 and /dev/xxx2 (e.g. /dev/ttyUSB0 and /dev/ttyUSB2),
and finally a third holding the remaining /dev/xxx1 and /dev/xxx3 devices. Each of
these three sets of devices is activated for a few seconds, and then the next set is
probed. This way, I ensure that kpilot never listens on /dev/ttyUSB0 and /dev/ttyUSB1
at the same time.

Now the first detection works fine. However, it seems the Linux kernel has another
problem with /dev/ttyUSB0. I have a Clie, which uses ttyUSB0, and as soon as the
wizard tries to listen on ttyUSB1 (after it detected the handheld on ttyUSB0 already),
the kernel writes a warning message to the syslog:
visor ttyUSB1: Device lied about number of ports, please use a lower one.

If I continue autodetection once again afterwards, the visor module kind of crashes.
lsmod shows an impossible usage count for the module:

reinhold@einstein:/kde/builddir$ lsmod
Module                  Size  Used by
visor                  17164  4294967294
usbserial              30704  1 visor

After that, the kernel doesn't detect the device ever again (until the computer is rebooted),
and the module can't be unloaded.
*/


ProbeDialog::ProbeDialog(QWidget *parent, const char *n) :
	KDialogBase(parent, n, true, i18n("Autodetecting Your Handheld"), KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::User1, KDialogBase::Cancel, true, i18n("Restart Detection")),
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

	fProgress = new KProgress( 100, fStatusGroup, "fProgress" );
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

	mDevicesToProbe[0] << "/dev/pilot";
	mDevicesToProbe[1] <<"/dev/ttyS0"<<"/dev/ttyS2"
	                <<"/dev/tts/0"<<"/dev/tts/2"
	                <<"/dev/ttyUSB0"<<"/dev/ttyUSB2"
	                <<"/dev/usb/tts/0"<<"/dev/usb/tts/2"
	                <<"/dev/cuaa0"<<"/dev/cuaa2"
	                <<"/dev/ucom0"<<"/dev/ucom2";
	mDevicesToProbe[2] <<"/dev/ttyS1"<<"/dev/ttyS3"
	                <<"/dev/tts/1"<<"/dev/tts/3"
	                <<"/dev/ttyUSB1"<<"/dev/ttyUSB3"
	                <<"/dev/usb/tts/1"<<"/dev/usb/tts/3"
	                <<"/dev/cuaa1"<<"/dev/cuaa3"
	                <<"/dev/ucom1"<<"/dev/ucom3";

	fProcessEventsTimer = new QTimer( this );
	fTimeoutTimer = new QTimer( this );
	fProgressTimer = new QTimer( this );
	fRotateLinksTimer = new QTimer( this );
	connect( fProcessEventsTimer, SIGNAL(timeout()), this, SLOT(processEvents()) );
	connect( fTimeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
	connect( fProgressTimer, SIGNAL(timeout()), this, SLOT( progress()) );
	connect( fRotateLinksTimer, SIGNAL(timeout()), this, SLOT( detect()) );
	connect( this, SIGNAL(finished()), this, SLOT(disconnectDevices()) );
}

ProbeDialog::~ProbeDialog()
{
}

void ProbeDialog::processEvents()
{
	FUNCTIONSETUP;
	KApplication::kApplication()->processEvents();
}

void ProbeDialog::progress()
{
	fProgress->advance(1);
}

int ProbeDialog::exec()
{
	mDetected = false;
	mUserName = "";
	mDevice = "";
	mUID = 0;
	QTimer::singleShot( 0, this, SLOT( startDetection() ) );
	return KDialogBase::exec();
}

void ProbeDialog::startDetection()
{
	FUNCTIONSETUP;

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
	if (!fTimeoutTimer->start( 30000, true ) ) 
		kdWarning()<<"Could not start fTimeoutTimer"<<endl;
	if (!fProcessEventsTimer->start( 100, false ) ) 
		kdWarning()<<"Could not start fProcessEventsTimer"<<endl;
	if (!fProgressTimer->start( 300, false) ) 
		kdWarning()<<"Could not start Progress timer"<<endl;

	KPilotDeviceLink*link;
	for (int i=0; i<3; i++)
	{
		QStringList::iterator end(mDevicesToProbe[i].end());
		for (QStringList::iterator it=mDevicesToProbe[i].begin(); it!=end; ++it)
		{
			link = new KPilotDeviceLink();
#ifdef DEBUG
			DEBUGKPILOT<<"new kpilotDeviceLink for "<<(*it)<<endl;
#endif
			link->reset( *it );
			link->close();
//			mDeviceLinkMap[*it] = link;
			mDeviceLinks[i].append( link );
			connect( link, SIGNAL(deviceReady(KPilotDeviceLink*)), this, SLOT(connection(KPilotDeviceLink*)) );
			processEvents();
		}
	}
	fStatus->setText( i18n("Waiting for handheld to connect...") );
	mProbeDevicesIndex=0;

	detect();
	if (!fRotateLinksTimer->start( 3000, false) ) 
		kdWarning()<<"Could not start Device link rotation timer"<<endl;
}


void ProbeDialog::detect(int i)
{
	FUNCTIONSETUP;
	PilotLinkList::iterator end(mDeviceLinks[mProbeDevicesIndex].end());
	for (PilotLinkList::iterator it=mDeviceLinks[mProbeDevicesIndex].begin(); it!=end; ++it)
	{
		if (*it) (*it)->close();
	}
	mProbeDevicesIndex = i;
	end=mDeviceLinks[mProbeDevicesIndex].end();
	for (PilotLinkList::iterator it=mDeviceLinks[mProbeDevicesIndex].begin(); it!=end; ++it)
	{
		if (*it) (*it)->reset();
	}
}

void ProbeDialog::detect()
{
	detect( (mProbeDevicesIndex+1)%3 );
}

void ProbeDialog::timeout()
{
	disconnectDevices();
	if (!mDetected) fStatus->setText( i18n("Timeout reached, could not detect a handheld.") );
}

void ProbeDialog::connection( KPilotDeviceLink*lnk)
{
	FUNCTIONSETUP;

	mActiveLink = lnk;
	if ( !mActiveLink ) return;
	KPilotUser*usr( mActiveLink->getPilotUser() );

	mUserName = usr->getUserName();
	mUID = usr->getUserID();
	mDevice = mActiveLink->pilotPath();

	fStatus->setText( i18n("Found a connected device on %1").arg(mDevice) );
	fUser->setText( mUserName );
	fDevice->setText( mDevice );
	mDetected = true;

	fResultsGroup->setEnabled( true );
	enableButtonOK(true);
	
	QTimer::singleShot(0, this, SLOT(retrieveDBList()));
}

void ProbeDialog::retrieveDBList()
{
	QPtrList<DBInfo> dbs = mActiveLink->getDBList();
	mDBs.clear();
	dbs.setAutoDelete( true );
	char buff[7];
	buff[0] = '[';

	DBInfo *dbi;
	for ( dbi = dbs.first(); dbi; dbi = dbs.next() ) {
		if ( dbi ) {
			set_long( &buff[1], dbi->creator );
			buff[5] = ']';
			buff[6] = '\0';
			QString cr( buff );
			mDBs << cr;
			dbi->name[33]='\0';
			mDBs << QString( dbi->name );
		}
	}
	mDBs.sort();
	
	QString old( QString::null ); 
	QStringList::Iterator itr = mDBs.begin();
	while ( itr != mDBs.end() ) {
		if ( old == *itr ) {
			itr = mDBs.remove( itr );
		} else {
			old = *itr;
			++itr;
		}
	}
	mActiveLink->endOfSync();

	QTimer::singleShot(0, this, SLOT(disconnectDevices()));
}
void ProbeDialog::disconnectDevices()
{
	FUNCTIONSETUP;

	if (!mDetected) fStatus->setText( i18n("Disconnected from all devices") );
	fProcessEventsTimer->stop( );
	fTimeoutTimer->stop();
	fProgressTimer->stop();
	fRotateLinksTimer->stop();
	fProgress->setProgress(fProgress->totalSteps());
	for (int i=0; i<3; ++i)
	{
		PilotLinkList::iterator end(mDeviceLinks[i].end());
		for (PilotLinkList::iterator it=mDeviceLinks[i].begin(); it!=end; ++it)
		{
			(*it)->close();
			KPILOT_DELETE(*it);
		}
		mDeviceLinks[i].clear();
	}


	PilotDaemonDCOP_stub *daemonStub = new PilotDaemonDCOP_stub("kpilotDaemon", "KPilotDaemonIface");
	if (daemonStub)
	{
		daemonStub->startListening();
	}
	KPILOT_DELETE(daemonStub);
}

