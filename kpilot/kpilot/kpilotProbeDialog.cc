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

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfigskeleton.h> 

#include "kpilotConfig.h"
#include "options.h"

#include "kpilotProbeDialog.moc"
#ifndef __PILOTDAEMONDCOP_STUB__
#include "pilotDaemonDCOP_stub.h"
#endif



ProbeDialog::ProbeDialog(QWidget *parent, const char *n) :
	KDialogBase(parent, n, true, i18n("Autodetecting your handheld"), KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Cancel, true), 
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

	
	
	fResultsGroup = new QGroupBox( i18n( "Detected values" ), mainWidget, "fResultsGroup" );
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
	
	mDevicesToProbe << "/dev/pilot"
	                <<"/dev/ttyS0"<<"/dev/ttyS1"<<"/dev/ttyS2"<<"/dev/ttyS3"
	                <<"/dev/tts/0"<<"/dev/tts/1"<<"/dev/tts/2"<<"/dev/tts/3"
	                <<"/dev/ttyUSB0"<<"/dev/ttyUSB1"<<"/dev/ttyUSB2"<<"/dev/ttyUSB3"
	                <<"/dev/usb/tts/0"<<"/dev/usb/tts/1"<<"/dev/usb/tts/2"<<"/dev/usb/tts/3"
	                <<"/dev/cuaa0"<<"/dev/cuaa1"<<"/dev/cuaa2"<<"/dev/cuaa3"
	                <<"/dev/ucom0"<<"/dev/ucom1"<<"/dev/ucom2"<<"/dev/ucom3";
}

ProbeDialog::~ProbeDialog()
{
}

int ProbeDialog::exec() {
	mDetected = false;
	mUserName = "";
	mDevice = "";
	mUID = 0;
	QTimer::singleShot( 0, this, SLOT( startDetection() ) );
	QTimer::singleShot( 5000, this, SLOT( timeout() ) );
	QTimer::singleShot( 10000, this, SLOT( connection() ) );

	return KDialogBase::exec();
}

// Devices to probe:
// Linux: /dev/pilot (symlink), /dev/ttyS* (serial + irda), /dev/tts/[012345...] (with devfs), 
//        /dev/ttyUSB*, /dev/usb/tts/[012345...]
// *BSD: /dev/pilot, /dev/cuaa[01]   (serial), /dev/ucom* (usb)

void ProbeDialog::startDetection() {
	fStatus->setText( i18n("Starting detection...") );
	PilotDaemonDCOP_stub *daemonStub = new PilotDaemonDCOP_stub("kpilotDaemon", "KPilotDaemonIface");
	if (daemonStub) {
		daemonStub->stopListening();
	}
	KPILOT_DELETE(daemonStub);
		
		// TODO: connect to all sockets, connect each to the connection() slot
		// TODO: start timer to poll devfs devices periodically
}

void ProbeDialog::timeout() {
	disconnectDevices();
	fStatus->setText( i18n("Timeout reached, could not detect a handheld.") );
}

void ProbeDialog::connection() {
//	fStatus->setText( i18n("Found a connected device") );
	
	// TODO: After we have the connection, query the username and uid and the device and
	mUserName = "Reinhold Kainhofer";
	mDevice = "/dev/pilot";
	mUID = 12345;
	
	
	disconnectDevices();
	fStatus->setText( i18n("Found a connected device") );
	fUser->setText( mUserName );
	fDevice->setText( mDevice );
	fDevice->setText("/dev/pilot");
	mDetected = true;
	
	fResultsGroup->setEnabled( true );
	enableButtonOK(true);
}

void ProbeDialog::disconnectDevices() {
	fStatus->setText( i18n("Disconnected from all devices") );
	// TODO: Really disconnect all devices we are listening on
	
	PilotDaemonDCOP_stub *daemonStub = new PilotDaemonDCOP_stub("kpilotDaemon", "KPilotDaemonIface");
	if (daemonStub) {
		daemonStub->startListening();
	}
	KPILOT_DELETE(daemonStub);
}

