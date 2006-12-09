#ifndef _KPILOT_KPILOTDEVICELINK_H
#define _KPILOT_KPILOTDEVICELINK_H
/* kpilotdevicelink.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "kpilotlink.h"

/** @file Definition of the device link class; implemented in kpilotlink.cc */

/** Implementation of the device link class for physical
*   handheld devices, which communicate with the PC
*   using DLP / SLP via the pilot-link library.
*/
class KDE_EXPORT KPilotDeviceLink : public KPilotLink
{
// friend class SyncAction;
friend class PilotSerialDatabase;
Q_OBJECT

/*
** Constructors and destructors.
*/
public:
	/**
	* Creates a pilot link that can sync to the pilot.
	*
	* Call reset() on it to start looking for a device.
	*/
	KPilotDeviceLink( QObject *parent = 0, const char *name = 0, const QString &tempDevice = QString::null );
	/** Destructor. This rudely ends the communication with the handheld. */
	virtual ~KPilotDeviceLink();


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
		PilotLinkError,
		WorkaroundUSB
		} LinkStatus;

	/** Get the status (state enum) of this link.
	* @return The LinkStatus enum for the link's current state.
	*/
	LinkStatus status() const { return fLinkStatus; } ;
	/** Get a human-readable string for the given status @p l. */
	static QString statusString(LinkStatus l);


	virtual QString statusString() const;
	virtual bool isConnected() const;
	virtual void reset( const QString & );
	virtual void close();
	virtual void reset();
	virtual bool tickle();
	virtual const KPilotCard *getCardInfo(int card);
	virtual void endOfSync();
	virtual void finishSync();
	virtual int openConduit();
	virtual int getNextDatabase(int index,struct DBInfo *);
	virtual int findDatabase(const char *name, struct DBInfo*,
		int index=0, unsigned long type=0, unsigned long creator=0);
	virtual bool retrieveDatabase(const QString &path, struct DBInfo *db);
	virtual DBInfoList getDBList(int cardno=0, int flags=dlpDBListRAM);
	virtual PilotDatabase *database( const QString &name );

protected:
	virtual bool installFile(const QString &, const bool deleteFile);
	virtual void addSyncLogEntryImpl( const QString &s );
	virtual int pilotSocket() const { return fCurrentPilotSocket; } ;


private:
	LinkStatus fLinkStatus;





public:

	/**
	* Special-cases. Call this after a reset to set device-
	* specific workarounds; the only one currently known
	* is the Zire 31/72 T5 quirk of doing a non-HotSync
	* connect when it's switched on.
	*/
	void setWorkarounds(bool usb)
	{
		fWorkaroundUSB = usb;
	} ;

	/**
	* sets an additional device, which should be tried as fallback
	* usefull for hotplug enviroments
	*/
	void setTempDevice( const QString &device );

private:
	bool fWorkaroundUSB;
	QTimer *fWorkaroundUSBTimer;

private slots:
	void workaroundUSB();

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
	bool open( QString device = QString::null );

	/**
	* Check for device permissions and existence, emitting
	* warnings for weird situations. This is primarily intended
	* to inform the user.
	*/
	void checkDevice();

	/**
	* Some messages are only printed once and are suppressed
	* after that. These are indicated by flag bits in
	* messages.
	*/
	enum { OpenMessage=1, OpenFailMessage=2 } ;
	int messages;
	int messagesMask;
	static const int messagesType;

	void shouldPrint(int,const QString &);



private:
	/**
	* Path with resolved symlinks, to prevent double binding
	* to the same device.
	*/
	QString fRealPilotPath;

	/**
	* For transient devices: how often have we tried pi_bind()?
	*/
	int fRetries;

	/**
	* Timers and Notifiers for detecting activity on the device.
	*/
	QTimer *fOpenTimer;
	QSocketNotifier *fSocketNotifier;
	bool fSocketNotifierActive;

	/**
	* Pilot-link library handles for the device once it's opened.
	*/
	int fPilotMasterSocket;
	int fCurrentPilotSocket;
	QString fTempDevice;

	/**
	* Handle cases where we can't accept or open the device,
	* and data remains available on the pilot socket.
	*/
	int fAcceptedCount;

private:
	class KPilotDeviceLinkPrivate;
} ;

#endif

