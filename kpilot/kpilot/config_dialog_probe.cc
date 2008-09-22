/* KPilot
**
** Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the logic for the device probing dialog.
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

#include "options.h"

#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kapplication.h>
#include <kconfigskeleton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <kvbox.h>

#include "kpilotdevicelink.h"
#include "pilotSysInfo.h"
#include "pilotUser.h"

#include "kpilotConfig.h"
#include "kpilot_daemon_interface.h"

#include "config_dialog_probe.moc"

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
	KDialog(parent),
	fResultsGroup(0L),
	fStatusLabel(0L),
	fUserNameLabel(0L),
	fDeviceNameLabel(0L),
	fProgress(0L),
	fDetected(false), fUserName(), fDeviceName()
{
	if (n)
	{
		setObjectName(n);
	}
	setCaption(i18n("Autodetecting Your Handheld"));
	setButtonText(Ok,i18n("Restart Detection"));
	setButtons(Ok|Cancel|User1);
	setDefaultButton(Cancel);
	setModal(true);
	FUNCTIONSETUP;

	KVBox *mainWidget = new KVBox();
	setMainWidget(mainWidget);

	QLabel *infoText = new QLabel( i18n( "KPilot is now trying to automatically detect the device of your handheld. Please press the hotsync button if you have not done so already." ), mainWidget );
	infoText->setObjectName( "fInfoText" );
	infoText->setWordWrap( true );

	QGroupBox *statusGroup = new QGroupBox( i18n("Status"), mainWidget );
	QHBoxLayout *statusGroupLayout = new QHBoxLayout;
	statusGroup->setLayout( statusGroupLayout );

	fStatusLabel = new QLabel( i18n("Autodetection not yet started..."), statusGroup );
	statusGroupLayout->addWidget( fStatusLabel );

	fProgress = new QProgressBar( statusGroup );
	fProgress->setMaximum(100);
	statusGroupLayout->addWidget( fProgress );



	fResultsGroup = new QGroupBox( i18n( "Detected Values" ), mainWidget );
	fResultsGroup->setEnabled( false );
	QGridLayout *resultsGroupLayout = new QGridLayout;
	fResultsGroup->setLayout( resultsGroupLayout );

	QLabel *userLabel = new QLabel( i18n( "Handheld user:" ), fResultsGroup );
	resultsGroupLayout->addWidget( userLabel, 0, 0 );

	QLabel *deviceLabel = new QLabel( i18n( "Device:" ), fResultsGroup );
	resultsGroupLayout->addWidget( deviceLabel, 1, 0 );

	fUserNameLabel = new QLabel( i18n("[Not yet known]"), fResultsGroup );
	fUserNameLabel->setObjectName( "fUser" );
	resultsGroupLayout->addWidget( fUserNameLabel, 0, 1 );

	fDeviceNameLabel = new QLabel( i18n("[Not yet known]"), fResultsGroup );
	fDeviceNameLabel->setObjectName( "fDevice" );
	resultsGroupLayout->addWidget( fDeviceNameLabel, 1, 1 );


	resize( QSize(459, 298).expandedTo(minimumSizeHint()) );
	enableButtonOk(false);

	fDevicesToProbe[0] << "/dev/pilot";
	fDevicesToProbe[1] << "/dev/ttyS0"<< "/dev/ttyS2"
	                << "/dev/tts/0"<< "/dev/tts/2"
	                << "/dev/ttyUSB0"<< "/dev/ttyUSB2"
	                << "/dev/usb/tts/0"<< "/dev/usb/tts/2"
	                << "/dev/cuaa0"<< "/dev/cuaa2"
			<< "/dev/cuad0"<< "/dev/cuad2"
	                << "/dev/ucom0"<< "/dev/ucom2";
	fDevicesToProbe[2] << "/dev/ttyS1"<< "/dev/ttyS3"
	                << "/dev/tts/1"<< "/dev/tts/3"
	                << "/dev/ttyUSB1"<< "/dev/ttyUSB3"
	                << "/dev/usb/tts/1"<< "/dev/usb/tts/3"
	                << "/dev/cuaa1"<< "/dev/cuaa3"
			<< "/dev/cuad1"<< "/dev/cuad3"
	                << "/dev/ucom1"<< "/dev/ucom3";

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
	FUNCTIONSETUP;
}

void ProbeDialog::processEvents()
{
	FUNCTIONSETUP;
	KApplication::kApplication()->processEvents();
}

void ProbeDialog::progress()
{
	fProgress->setValue(fProgress->value()+1);
}

int ProbeDialog::exec()
{
	fDetected = false;
	fUserName.clear();
	fDeviceName.clear();
	QTimer::singleShot( 0, this, SLOT( startDetection() ) );
	return KDialog::exec();
}

void ProbeDialog::startDetection()
{
	FUNCTIONSETUP;

	disconnectDevices();
	fProgress->setValue(0);
	fStatusLabel->setText( i18n("Starting detection...") );
	OrgKdeKpilotDaemonInterface *daemonInterface = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
	if (daemonInterface) {
		daemonInterface->stopListening();
	}
	KPILOT_DELETE(daemonInterface);
	fTimeoutTimer->setSingleShot( true );
	fTimeoutTimer->start( 30000 );
	fProcessEventsTimer->setSingleShot( false );
	fProcessEventsTimer->start( 100 );
	fProgressTimer->setSingleShot( false );
	fProgressTimer->start( 300 );

	KPilotDeviceLink*link;
	for (int i=0; i<3; ++i)
	{
		QStringList::iterator end(fDevicesToProbe[i].end());
		for (QStringList::iterator it=fDevicesToProbe[i].begin(); it!=end; ++it)
		{
			link = new KPilotDeviceLink();
			link->setDevice((*it));
			DEBUGKPILOT << "new kpilotDeviceLink for " << (*it);
			fDeviceLinks[i].append( link );
			connect( link, SIGNAL(deviceReady(KPilotLink*)), this, SLOT(connection(KPilotLink*)) );
			processEvents();
		}
	}
	fStatusLabel->setText( i18n("Waiting for handheld to connect...") );
	fProbeDevicesIndex=0;

	detect();
	fRotateLinksTimer->setSingleShot( false );
	fRotateLinksTimer->start( 3000 );
}


void ProbeDialog::detect(int i)
{
	FUNCTIONSETUP;

	fProbeDevicesIndex = i;
	PilotLinkList::iterator end(fDeviceLinks[fProbeDevicesIndex].end());

	for (PilotLinkList::iterator it=fDeviceLinks[fProbeDevicesIndex].begin(); it!=end; ++it)
	{
		if (*it)
		{
			(*it)->reset();
		}
	}
}

void ProbeDialog::detect()
{
	detect( (fProbeDevicesIndex+1)%3 );
}

void ProbeDialog::timeout()
{
	disconnectDevices();
	if (!fDetected) {
		fStatusLabel->setText( i18n("Timeout reached, could not detect a handheld.") );
		KMessageBox::information ( this
			, i18n("<qt>"
				"<p>A handheld could not be detected. Possible check the following "
				"things:</p>"
				"<ul>"
				"<li>Have you pressed the hotsync button on the handheld?</li>"
				"<li>Make sure the device sits in the cradle correctly.</li>"
				"<li>Make sure the cradle is correctly plugged in to the computer.</li>"
				"<li>Have you checked that your device is actually supported by kpilot"
				" (see http://www.kpilot.org).</li>"
				"</ul></qt>"
			), i18n("Automatic Detection Failed"), "AutoDetectionFailed");
	}
}

void ProbeDialog::connection( KPilotLink *lnk)
{
	FUNCTIONSETUP;

	fActiveLink = static_cast<KPilotDeviceLink*>(lnk);
	if ( !fActiveLink )
	{
		return;
	}
	const KPilotUser &usr( fActiveLink->getPilotUser() );

	fUserName = usr.name();
	fDeviceName = fActiveLink->pilotPath();

	fStatusLabel->setText( i18n("Found a connected device on %1",fDeviceName) );
	fUserNameLabel->setText( fUserName );
	fDeviceNameLabel->setText( fDeviceName );
	fDetected = true;

	fResultsGroup->setEnabled( true );
	enableButtonOk(true);

	QTimer::singleShot(0, this, SLOT(retrieveDBList()));
}

void ProbeDialog::retrieveDBList()
{
	KPilotLink::DBInfoList dbs = fActiveLink->getDBList();
	fDBs.clear();
	char buff[7];
	buff[0] = '[';

	for ( KPilotLink::DBInfoList::ConstIterator i = dbs.begin(); i != dbs.end(); ++i )
	{
		set_long( &buff[1], (*i).creator );
		buff[5] = ']';
		buff[6] = '\0';
		QString cr( buff );
		fDBs << cr;
		fDBs << QString( (*i).name );
	}
	fDBs.sort();

	QString old;
	QStringList::Iterator itr = fDBs.begin();
	while ( itr != fDBs.end() )
	{
		if ( old == *itr )
		{
			itr = fDBs.erase( itr );
		}
		else
		{
			old = *itr;
			++itr;
		}
	}

	// End sync gracefully, but don't change settings on the handheld.
	fActiveLink->endSync( KPilotLink::NoUpdate );

	QTimer::singleShot(0, this, SLOT(disconnectDevices()));
}
void ProbeDialog::disconnectDevices()
{
	FUNCTIONSETUP;

	if (!fDetected)
	{
		fStatusLabel->setText( i18n("Disconnected from all devices") );
	}
	fProcessEventsTimer->stop( );
	fTimeoutTimer->stop();
	fProgressTimer->stop();
	fRotateLinksTimer->stop();
	fProgress->setValue(fProgress->maximum());
	for (int i=0; i<3; ++i)
	{
		PilotLinkList::iterator end(fDeviceLinks[i].end());
		for (PilotLinkList::iterator it=fDeviceLinks[i].begin(); it!=end; ++it)
		{
			(*it)->close();
			KPILOT_DELETE(*it);
		}
		fDeviceLinks[i].clear();
	}


        OrgKdeKpilotDaemonInterface *daemonInterface = new OrgKdeKpilotDaemonInterface("org.kde.kpilot.daemon", "/Daemon", QDBusConnection::sessionBus());
	if (daemonInterface)
	{
		daemonInterface->startListening();
	}
	KPILOT_DELETE(daemonInterface);
}

