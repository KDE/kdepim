/* kpilotlink.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a horrible class that implements three different
** functionalities and that should be split up as soon as 2.1 
** is released. See the .cc file for more.
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef _KPILOT_KPILOTLINK_H
#define _KPILOT_KPILOTLINK_H

#include <pi-dlp.h>

#ifndef QOBJECT_H
#include <qobject.h>
#endif


class QTimer;
class QSocketNotifier;
class KPilotUser;

/*
** The KPilotLink class was originally a kind of C++ wrapper
** for the pilot-link library. It grew and grew and mutated
** until it was finally cleaned up again in 2001. In the meantime
** it had become something that wrapped a lot more than just
** pilot-link. This class currently does:
** 
** * Client (ie. conduit) handling of kpilotlink protocol connections
** * Pilot-link handling
**
** Which is exactly what is needed: something that conduits can
** plug onto to talk to the pilot.
*/


/*
** The struct db is a description class for Pilot databases
** by Kenneth Albanowski. It's not really clear why it's *here*.
** The macro pi_mktag is meant to be given four char (8-bit)
** quantities, which are arranged into an unsigned long; for example
** pi_mktag('l','n','c','h'). This is called the creator tag
** of a database, and db.creator can be compared with such a
** tag. The tag lnch is used by the Pilot's launcher app. Some
** parts of KPilot require such a tag.
*/
struct db 
{
	char name[256];
	int flags;
	unsigned long creator;
	unsigned long type;
	int maxblock;
};

#define pi_mktag(c1,c2,c3,c4) (((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4))




/*
** KPilotLink is a QObject so that it can use the Qt signals / slots
** architecture (although it currently does so only half-heartedly).
*/

class KPilotLink : public QObject
{
protected:
	KPilotLink(const char *name) : QObject(0L,name) { } ;


public:
	/**
	* This belongs on both client and server end: these are
	* commands sent from KPilot telling the daemon
	* to perform those actions.
	*/
	enum Commands { Backup, 
		Restore, 
		HotSync, 
		FastSync,
		InstallFile, 
		TestConnection 
	};

	virtual QString statusString() const = 0;
} ;

class KPilotDeviceLink : public KPilotLink
{
Q_OBJECT

/*
** Constructors and destructors.
*/
protected:
	/**
	* Creates a pilot link that can sync to the pilot.
	*
	* Call reset() on it to start looking for a device.
	*/
	KPilotDeviceLink();
private:
	static KPilotDeviceLink *fDeviceLink;

public:
	virtual ~KPilotDeviceLink();


/*
** Status information
*/

public:
	/**
	* The link behaves like a state machine most of the time:
	* it waits for the actual device to become available, and
	* then becomes ready to handle syncing.
	*/
	typedef enum {
		Init,
		WaitingForDevice,
		FoundDevice,
		CreatedSocket,
		DeviceOpen,
		AcceptedDevice,
		SyncDone,
		PilotLinkError
		} LinkStatus;

	LinkStatus status() const { return fStatus; } ;
	virtual QString statusString() const;

	/**
	* True if HotSync has been started but not finished yet
	* (ie. the physical Pilot is waiting for sync commands)
	*/
	bool getConnected() const { return fStatus == AcceptedDevice; }

private:
	LinkStatus fStatus;


/*
** Used for initially attaching to the device.
** deviceReady() is emitted when the device has been opened
** and a Sync can start.
*/
public:
	/**
	* Return the device link to the Init state and try connecting
	* to the given device path (if it's non-empty).
	*/
	void reset(const QString &pilotPath = QString::null);

	/**
	* Release all resources, including the master pilot socket,
	* timers, notifiers, etc.
	*/
	void close();

	/**
	* All sorts of device and connection information.
	*/
	QString pilotPath() const { return fPilotPath; } ;
	bool isTransient() const { return fTransientDevice; } ;
	void setTransient(bool b) { fTransientDevice=b; } ;

protected slots:
	/**
	* Attempt to open the device. Called regularly to check
	* if the device exists (to handle USB-style devices).
	*/
	void openDevice();

	/**
	* Called when the device is opened *and* activity occurs on the
	* device. This indicates the beginning of a hotsync.
	*/
	void acceptDevice();

protected:
	/**
	* Does the low-level opening of the device and handles the
	* pilot-link library initialisation.
	*/
	bool open();

signals:
	/**
	* Emitted once the user information has been read and
	* the HotSync is really ready to go.
	*/
	void deviceReady();

protected:
	int pilotSocket() const { return fCurrentPilotSocket; } ;


private:
	/**
	* Path of the device special file that will be used.
	* Usually /dev/pilot, /dev/ttySx, or /dev/usb/x.
	*/
	QString fPilotPath;

	/**
	* Is this a transient (USB) style device?
	*/
	bool fTransientDevice;

	/**
	* For transient devices: how often have we tried pi_bind()?
	*/
	int fRetries;

	/**
	* Timers and Notifiers for detecting activity on the device.
	*/
	QTimer *fOpenTimer;
	QSocketNotifier *fSocketNotifier;

	/**
	* Pilot-link library handles for the device once it's opened.
	*/
	int fPilotMasterSocket;
	int fCurrentPilotSocket;

/*
** Utility functions for during a Sync.
*/
public:
 	/**
 	* Write a log entry to the pilot. Note that the library
 	* function takes a char *, not const char * (which is
 	* highly dubious). Causes signal logEntry(const char *)
 	* to be emitted.
 	*/
 	void addSyncLogEntry(const QString &entry);

signals:
 	/**
 	* Whenever a conduit adds a Sync log entry (actually,
 	* KPilotLink itself adds some log entries itself),
	* this signal is emitted.
	*/
 	void logMessage(const QString &);
 	void logError(const QString &);
 	void logProgress(const QString &, int);


/*
** Pilot User Identity functions.
*/
public:
	/**
	* Returns the user information as set in the KPilot settings dialog.
	* The user information can also be set by the Pilot, and at the
	* end of a HotSync the two user informations are synced as well.
	*/
	KPilotUser *getPilotUser() { return fPilotUser; }

protected:
	KPilotUser  *fPilotUser;
} ;


#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif

// $Log$
// Revision 1.29  2001/09/06 22:04:27  adridg
// Enforce singleton-ness & retry pi_bind()
//
// Revision 1.28  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
