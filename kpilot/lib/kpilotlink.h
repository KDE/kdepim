#ifndef _KPILOT_KPILOTLINK_H
#define _KPILOT_KPILOTLINK_H
/* kpilotlink.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Encapsulates all the communication with the pilot. Also
** does daemon-like polling of the Pilot. Interesting status
** changes are signalled.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <time.h>
#include <pi-dlp.h>

#ifndef QOBJECT_H
#include <qobject.h>
#endif


class QTimer;
class QDateTime;
class QSocketNotifier;
class KPilotUser;
struct DBInfo;

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

/**
 * Convert a struct tm from the pilot-link package to a QDateTime
 */
QDateTime readTm(const struct tm &t);
/**
 * Convert a QDateTime to a struct tm for use with the pilot-link package
 */
struct tm writeTm(const QDateTime &dt);


class KPilotDeviceLink : public QObject
{
friend class SyncAction;
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
	KPilotDeviceLink(QObject *parent, const char *name);
private:
	static KPilotDeviceLink *fDeviceLink;

public:
	virtual ~KPilotDeviceLink();
	static KPilotDeviceLink *link() { return fDeviceLink; } ;
	static KPilotDeviceLink *init(QObject *parent=0L,const char *n=0L);

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
	
public slots:
	/**
	* Tickle the palm to reset the timeout
	*/
	void tickle() const;

private:
	LinkStatus fStatus;


/*
** Used for initially attaching to the device.
** deviceReady() is emitted when the device has been opened
** and a Sync can start.
*/
public:
	/**
	* Information on what kind of device we're dealing with.
	*/
	typedef enum { None,
		Serial,
		OldStyleUSB,
		DevFSUSB
		} DeviceType;

	DeviceType deviceType() const { return fDeviceType; } ;
	QString deviceTypeString(int i) const;
	bool isTransient() const
	{
		return (fDeviceType==OldStyleUSB) ||
			(fDeviceType==DevFSUSB);
	}

	QString pilotPath() const { return fPilotPath; } ;

	/**
	* Return the device link to the Init state and try connecting
	* to the given device path (if it's non-empty).
	*/
	void reset(DeviceType t,const QString &pilotPath = QString::null);


public slots:
	/**
	* Release all resources, including the master pilot socket,
	* timers, notifiers, etc.
	*/
	void close();

	/**
	* Assuming things have been set up at least once already by
	* a call to reset() with parameters, use this slot to re-start
	* with the same settings.
	*/
	void reset();

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
	* What kind of device is this?
	*/
	DeviceType fDeviceType;

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

signals:
	/**
	* Whenever a conduit adds a Sync log entry (actually,
	* KPilotLink itself adds some log entries itself),
	* this signal is emitted.
	*/
	void logEntry(const char *);

/*
** File installation.
*/
public:
	int installFiles(const QStringList &, const bool deleteFiles=true);
protected:
	bool installFile(const QString &, const bool deleteFile=true);

 	/**
 	* Write a log entry to the pilot. Note that the library
 	* function takes a char *, not const char * (which is
 	* highly dubious). Causes signal logEntry(const char *)
 	* to be emitted if @p log is true.
 	*/
 	void addSyncLogEntry(const QString &entry,bool log=true);

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
	* end of a HotSync the two user informations can be synced as well
	* with finishSync -- this writes fPilotUser again, so don't make
	* local copies of the KPilotUser structure and modify them.
	*/
	KPilotUser *getPilotUser() { return fPilotUser; }
	void finishSync();

protected:
	KPilotUser  *fPilotUser;

/*
** Actions intended just to abstract away the pilot-link library interface.
*/
protected:
	/**
	* Notify the Pilot user which conduit is running now.
	*/
	int openConduit();
public:
	int getNextDatabase(int index,struct DBInfo *);
	int findDatabase(const char *name, struct DBInfo*, 
		int index=0, long type=0, long creator=0);

	/**
	* Retrieve the database indicated by DBInfo *db into the
	* local file @p path.
	*/
	bool retrieveDatabase(const QString &path, struct DBInfo *db);
	
public:
	/**
	 * Get the time from the handheld device into a QDateTime
	 */
	QDateTime getTime();
	/**
	 * Set the time on the handheld to the give QDateTime
	 */
	bool setTime(const time_t &pctime);
	
	/**
	 * Get the version number from the handheld
	 */
	unsigned long ROMversion() const;
	/**
	 * Get the major PalmOS version number
	 */
	unsigned long majorVersion() const;
	/**
	 * Get the minor PalmOS version number
	 */
	unsigned long minorVersion() const;
	 
	
} ;

bool operator < ( const struct db &, const struct db &) ;

#endif
