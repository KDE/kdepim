#ifndef _KPILOT_KPILOTLINK_H
#define _KPILOT_KPILOTLINK_H
/* kpilotlink.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
class QThread;
class KPilotUser;
class KPilotSysInfo;
class KPilotCard;
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



class KPilotDeviceLink : public QObject
{
friend class SyncAction;
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
	virtual ~KPilotDeviceLink();
//	bool init(QObject *parent=0L,const char *n=0L);


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

	LinkStatus status() const { return fLinkStatus; } ;
	virtual QString statusString() const;

	/**
	* True if HotSync has been started but not finished yet
	* (ie. the physical Pilot is waiting for sync commands)
	*/
	bool getConnected() const { return fLinkStatus == AcceptedDevice; }

private:
	LinkStatus fLinkStatus;

/**
* Tickle handling.
*
* During a HotSync, the Pilot expects to be kept awake by (nearly)
* continuous communication with the PC. The Pilot doesn't like
* long periods of inactivity, since they drain the batteries while
* the communications hardware is kept powered up. If the period of
* inactivity is too long, the Pilot times out, shuts down the
* communication, and the HotSync is broken.

* Sometimes, however, periods of inactivity cannot be avoided --
* for instance, if you _have_ to ask the user something during a
* sync, or if you are fetching a large amount of data from a slow
* source (libkabc can do that, if your addressbook is on an LDAP
* server). During these periods of inactivity (as far as the Pilot
* can tell), you can "tickle" the Pilot to keep it awake. This
* prevents the communications from being shut down. It's not
* a good idea to do this all the time -- battery life and possible
* corruption of the dlp_ communications streams. Hence, you should
* start and stop tickling the Pilot around any computation which:
*   - may take a long time
*   - does not in itself _ever_ communicate directly with the Pilot
*
*
*
* You can call slot tickle() whenever you like just to do a
* dlp_tickle() call on the Pilot. It will return true if the
* tickle was successful, false otherwise (this can be used to
* detect if the communication with the Pilot has shut down for
* some reason).
*
* The protected methods startTickle() and stopTickle() are intended
* to be called only from SyncActions -- I can't think of any other
* legitimate use, since everything being done during a HotSync is
* done via subclasses of SyncActions anyway, and SyncAction provides
* access to these methods though its own start- and stopTickle().
*
* Call startTickle with a timeout in seconds, or 0 for no timeout.
* This timeout is _unrelated_ to the timeout in the Pilot's
* communications. Instead, it indicates how long to continue
* tickling the Pilot before emitting the timeout() signal. This
* can be useful for placing an upper bound on the amount of
* time to wait for, say, user interaction -- you don't want an
* inattentive user to drain the batteries during a sync because
* he doesn't click on "Yes" for some question. If you pass a
* timeout of 0, the Pilot will continue to be tickled until you
* call stopTickle().
*
* Call stopTickle() to stop tickling the Pilot and continue with
* normal operation. You _must_ call stopTickle() before calling
* anything else that might communicate with the Pilot, to avoid
* corrupting the dlp_ communications stream. (TODO: Mutex the heck
* out of this to avoid this problem). Note that stopTickle() may
* hang up the caller for a small amount of time (up to 200ms)
* before returning.
*
* event() and TickleTimeoutEvent are part of the implementation
* of tickling, and are only accidentally visible.
*
* Signal timeout() is emitted if startTickle() has been called
* with a non-zero timeout and that timeout has elapsed. The
* tickler is stopped before timeout is emitted.
*/
public slots:
	bool tickle() const;
protected:
	void startTickle(unsigned int timeout=0);
	void stopTickle();
public:
	virtual bool event(QEvent *e);
	static const unsigned int TickleTimeoutEvent = 1066;

signals:
	void timeout();

private:
	bool fTickleDone;
	QThread *fTickleThread;





/*
** Used for initially attaching to the device.
** deviceReady(KPilotDeviceLink*) is emitted when the device has been opened
** and a Sync can start.
*/
public:
	/**
	* Information on what kind of device we're dealing with.
	*/
	QString pilotPath() const { return fPilotPath; } ;

	/**
	* Return the device link to the Init state and try connecting
	* to the given device path (if it's non-empty).
	*/
	void reset(const QString &pilotPath);

	/**
	* sets an additional device, which should be tried as fallback
	* usefull for hotplug enviroments
	*/
	void setTempDevice( const QString &device );


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

signals:
	/**
	* Emitted once the user information has been read and
	* the HotSync is really ready to go.
	*/
	void deviceReady( KPilotDeviceLink* );

protected:
	int pilotSocket() const { return fCurrentPilotSocket; } ;


private:
	/**
	* Path of the device special file that will be used.
	* Usually /dev/pilot, /dev/ttySx, or /dev/usb/x.
	*/
	QString fPilotPath;
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
protected:
	KPilotUser  *fPilotUser;
	KPilotSysInfo *fPilotSysInfo;
public:
	/**
	* Returns the user information as set in the KPilot settings dialog.
	* The user information can also be set by the Pilot, and at the
	* end of a HotSync the two user informations can be synced as well
	* with finishSync -- this writes fPilotUser again, so don't make
	* local copies of the KPilotUser structure and modify them.
	*/
	KPilotUser *getPilotUser() { return fPilotUser; }
	KPilotSysInfo *getSysInfo() { return fPilotSysInfo; }
	KPilotCard *getCardInfo(int card=0);
	void endOfSync();
	void finishSync();

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
	QPtrList<DBInfo> getDBList(int cardno=0, int flags=dlpDBListRAM);

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

private:
	class KPilotDeviceLinkPrivate;
} ;

bool operator < ( const struct db &, const struct db &) ;

#endif
