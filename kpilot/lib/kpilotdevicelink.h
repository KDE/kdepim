#ifndef _KPILOT_KPILOTDEVICELINK_H
#define _KPILOT_KPILOTDEVICELINK_H
/*
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

class QThread;

class DeviceMap; ///< Globally tracks all devices that have a link assigned
class Messages; ///< Tracks which messages have been printed
class DeviceCommThread; ///< Thread for doing all palm device communications

/**
* The link behaves like a state machine most of the time:
* it waits for the actual device to become available, and
* then becomes ready to handle syncing.
*/
enum LinkStatus {
	Init,
	WaitingForDevice,
	FoundDevice,
	CreatedSocket,
	DeviceOpen,
	AcceptedDevice,
	SyncDone,
	PilotLinkError,
	WorkaroundUSB
} ;

/**
* Custom events we can be handling...
*/
enum DeviceCustomEvents { 
	EventLogMessage = QEvent::User + 777,
	EventLogError,
	EventLogProgress,
	EventDeviceReady
};
		 
/**
* Definition of the device link class for physical
* handheld devices, which communicate with the PC
* using DLP / SLP via the pilot-link library.
*/
class KDE_EXPORT KPilotDeviceLink : public KPilotLink
{
friend class PilotSerialDatabase;
friend class DeviceCommThread;

Q_OBJECT

public:
	/**
	* Constructor. Creates a link that can sync to a physical handheld.
	* Call reset() on it to start looking for a device.
	*
	* @param parent Parent object.
	* @param name   Name of this object.
	* @param tempDevice Path to device node to use as an alternative
	*                   to the "normal" one set by KPilot.
	*/
	KPilotDeviceLink( QObject *parent = 0,
		const char *name = 0,
		const QString &tempDevice = QString::null );

	/**
	* Destructor. This rudely ends the communication with the handheld.
	* It is best to call endOfSync() or finishSync() before destroying
	* the device.
	*/
	virtual ~KPilotDeviceLink();

	/**
	* Get the status (state enum) of this link.
	* @return The LinkStatus enum for the link's current state.
	*/
	LinkStatus status() const
	{
		return fLinkStatus;
	}

	/** Get a human-readable string for the given status @p l. */
	static QString statusString(LinkStatus l);

	// The following API is the actual implementation of
	// the KPilotLink API, for documentation see that file.
	//
	virtual QString statusString() const;
	virtual bool isConnected() const;
	virtual void reset( const QString & );
	virtual void close();
	virtual void reset();
	virtual bool event(QEvent *e);
	virtual bool tickle();
	virtual const KPilotCard *getCardInfo(int card);
	virtual void endSync( EndOfSyncFlags f );
	virtual int openConduit();
	virtual int getNextDatabase(int index,struct DBInfo *);
	virtual int findDatabase(const char *name, struct DBInfo*,
		int index=0, unsigned long type=0, unsigned long creator=0);
	virtual bool retrieveDatabase(const QString &path, struct DBInfo *db);
	virtual DBInfoList getDBList(int cardno=0, int flags=dlpDBListRAM);
	virtual PilotDatabase *database( const QString &name );
	virtual PilotDatabase *database( const DBInfo *info );

protected:
	virtual bool installFile(const QString &, const bool deleteFile);
	virtual void addSyncLogEntryImpl( const QString &s );
	virtual int pilotSocket() const
	{
		return fPilotSocket;
	}


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
	}

	/**
	* Sets an additional device, which should be tried as fallback.
	* Useful for hotplug enviroments, this device is used @em once
	* for accepting a connection.
	*/
	void setTempDevice( const QString &device );
	
	/**
	* Sets the device to use.  Used by probe dialog, since we know
	* what device to use, but we don't want to start the detection
	* immediately.
	*/
	void setDevice( const QString &device ) 
	{ 
		fPilotPath = device;
	}
	

protected:
	/** Should we work around the Zire31/72 quirk? @see setWorkarounds() */
	bool fWorkaroundUSB;


	/**
	* Check for device permissions and existence, emitting
	* warnings for weird situations. This is primarily intended
	* to inform the user.
	*/
	void checkDevice();

protected:
	/**
	* Path with resolved symlinks, to prevent double binding
	* to the same device.
	*/
	QString fRealPilotPath;

	/**
	* Pilot-link library handles for the device once it's opened.
	*/
	int fPilotSocket;
	QString fTempDevice;

	/**
	* Handle cases where we can't accept or open the device,
	* and data remains available on the pilot socket.
	*/
	int fAcceptedCount;
	
	/**
	* Start/Stop our device communication thread.
	*/
	void startCommThread();
	void stopCommThread();

protected:
	Messages *fMessages;
	DeviceCommThread *fDeviceCommThread;
} ;

#endif

