#ifndef _KPILOT_KPILOTLINK_H
#define _KPILOT_KPILOTLINK_H
/* kpilotlink.h			KPilot
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

#include <time.h>
#include <pi-dlp.h>

#include <qobject.h>
#include <qvaluelist.h>

class QTimer;
class QDateTime;
class QSocketNotifier;
class QThread;
class KPilotUser;
class KPilotSysInfo;
class KPilotCard;
class PilotDatabase;
struct DBInfo;



/** @file
** Encapsulates all the communication with the pilot. Also
** does daemon-like polling of the Pilot. Interesting status
** changes are signalled.
**
** This file defines three classes; these handle some aspects of
** communication with a Handheld. A KPilotLink object represents a
** connection to a device (which may be active or inactive -- the latter in
** cases where the link is @e waiting for a device to show up). The object
** handles waiting, protocol initialization and some general
** tasks such as getting system information or user data.
**
** The actual communication with the handheld should use the
** PilotDatabase methods or use pilot-link dlp_* functions directly
** on handle().
**
** The KPilotLink class was originally a kind of C++ wrapper
** for the pilot-link library. It grew and grew and mutated
** until it was finally cleaned up again in 2001. In the meantime
** it had become something that wrapped a lot more than just
** pilot-link.
**
** This file defines an abstract base class KPilotLink, which may
** be specialized for a real physical device that communicates
** with DLP/SLP through the pilot-link library, or as a "fake"
** device which uses data on the local filesystem to simulate
** a device. These subclasses are KPilotDeviceLink and KPilotLocalLink.
*/

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

/** A list of DBInfo structures. */
typedef QValueList<struct DBInfo> DBInfoList;

/** This is the abstract base class for Handheld interaction.
*   It tries to define all the behavior that we need; calls
*   to virtual *Impl() functions are used to implement the
*   specific behavior for subclasses.
*/
class KDE_EXPORT KPilotLink : public QObject
{
Q_OBJECT
friend class SyncAction;
public:
	/** Constructor. Use reset() to start looking for a device. */
	KPilotLink( QObject *parent = 0, const char *name = 0 );
	/** Destructor. This rudely interrupts any communication in progress. */
	virtual ~KPilotLink();


	/** Provides a human-readable status string. */
	virtual QString statusString() const = 0;

	/**
	* True if HotSync has been started but not finished yet
	* (ie. the physical Pilot is waiting for sync commands)
	*/
	virtual bool isConnected() const = 0;


	/**
	* Information on what kind of device we're dealing with.
	* A link is associated with a path -- either the node in
	* /dev that the physical device is attached to, or an
	* IP address, or a filesystem path for local links.
	* Whichever is being used, this function returns its
	* name in a human-readable form.
	*/
	QString pilotPath() const { return fPilotPath; } ;

	/**
	* Return the device link to the Init state and try connecting
	* to the given device path (if it's non-empty). What the
	* path means depends on the kind of link we're instantiating.
	*
	* @see reset()
	* @see pilotPath()
	*/
	virtual void reset(const QString &pilotPath) = 0;

	/** Implementation detail to handle tickle timeouts. */
	virtual bool event(QEvent *e);

	/** Install the list of files (full paths!) named by @p l
	*   onto the handheld (or whatever this link represents).
	*   If @p deleteFiles  is true, the source files are removed.
	*
	*   @return the number of files successfully installed.
	*/
	unsigned int installFiles(const QStringList &l, const bool deleteFiles);

	/**
	* Write a log entry to the handheld. If @p log is true,
	* then the signal logMessage() is also emitted. This
	* function is supposed to @em only write to the handheld's
	* log (with a physical device, that is what appears on
	* screen at the end of a sync).
	*/
	void addSyncLogEntry(const QString &entry,bool log=true);

	/** Find a database with the given @p name (and optionally,
	*   type @p type and creator ID (from pi_mktag) @p creator,
	*   on searching from index @p index on the handheld.
	*   Fills in the DBInfo structure if found, returns < 0
	*   on error.
	*/
	virtual int findDatabase(const char *name, struct DBInfo*,
		int index=0, unsigned long type=0, unsigned long creator=0) = 0;

	/**
	* Retrieve the database indicated by DBInfo @p *db into the
	* local file @p path. This copies all the data, and you can
	* create a PilotLocalDatabase from the resulting @p path .
	*
	* @return @c true on success
	*/
	virtual bool retrieveDatabase(const QString &path, struct DBInfo *db) = 0;

	/** Fill the DBInfo structure @p db with information about
	*   the next database (in some ordering) counting from
	*   @p index.
	*   @return < 0 on error
	*/
	virtual int getNextDatabase(int index,struct DBInfo *db) = 0;

	/** Returns a list of DBInfo structures describing all the
	*   databases available on the link (ie. device) with the
	*   given card number @p cardno and flags @p flags. No known
	*   handheld uses a cardno other than 0; use flags to
	*   indicate what kind of databases to fetch -- @c dlpDBListRAM
	*   or @c dlpDBListROM.
	*
	*   @return list of DBInfo objects, one for each database
	*   @note ownership of the DBInfo objects is passed to the
	*         caller, who must delete the objects.
	*/
	virtual DBInfoList getDBList(int cardno=0, int flags=dlpDBListRAM) = 0;

	/** Return a database object for manipulating the database with
	*    name @p name on the link. This database may be local or
	*    remote, depending on the kind of link in use.
	*
	*    @return pointer to database object, or 0 on error.
	*    @note ownership of the database object is given to the caller,
	*          who must delete the object in time.
	*/
	virtual PilotDatabase *database( const QString &name ) = 0;

	/**
	* Retrieve the user information from the device. Ownership
	* is kept by the link, and at the end of a sync the user
	* information is synced back to the link -- so it may be
	* modified, but don't make local copies of it.
	*
	* @note Do not call this before the sync begins!
	*/
	KPilotUser &getPilotUser() { return *fPilotUser; }

	/**
	* System information about the handheld. Ownership is kept
	* by the link. For non-device links, something fake is
	* returned.
	*
	* @note Do not call this before the sync begins!
	*/
	const KPilotSysInfo &getSysInfo() { return *fPilotSysInfo; }

	/**
	* Retrieve information about the data card @p card;
	* I don't think that any pilot supports card numbers
	* other than 0. Non-device links return something fake.
	*
	* This function may return NULL (non-device links or
	* on error).
	*
	* @note Ownership of the KPilotCard object is given
	*       to the caller, who must delete it.
	*/
	virtual const KPilotCard *getCardInfo(int card=0) = 0;

	/** End the sync in a gracuful manner. */
	virtual void endOfSync() = 0;
	/** End the sync in a graceful manner @em and update
	*   the last-sync time and user information on the handheld.
	*/
	virtual void finishSync() = 0;

signals:
	/** A timeout associated with tickling has occurred. Each
	*   time startTickle() is called, you can state how long
	*   tickling should last (at most) before timing out.
	*
	*   You can only get a timeout when the Qt event loop is
	*   running, which somewhat limits the usefulness of timeouts.
	*/
	void timeout();

	/** Signal that a message has been written to the sync log. */
	void logMessage(const QString &);
	/** Signal that an error has occurred, for logging. */
	void logError(const QString &);
	/** Signal that progress has been made, for logging purposes.
	*   @p p is the percentage completed (0 <= s <= 100).
	*   The string @p s is logged as well, if non-Null.
	*/
	void logProgress(const QString &s, int p);

	/**
	* Emitted once the user information has been read and
	* the HotSync is really ready to go.
	*/
	void deviceReady( KPilotLink* );


public slots:
	/**
	* Release all resources, including the master pilot socket,
	* timers, notifiers, etc.
	*/
	virtual void close() = 0;

	/**
	* Assuming things have been set up at least once already by
	* a call to reset() with parameters, use this slot to re-start
	* with the same settings.
	*/
	virtual void reset() = 0;

	/** Tickle the underlying device exactly once. */
	virtual bool tickle() = 0;

protected:
	/**
	* Path of the device special file that will be used.
	* Usually /dev/pilot, /dev/ttySx, or /dev/usb/x. May be
	* a filesystem path for local links.
	*/
	QString fPilotPath;

	/** Start tickling the Handheld (every few seconds). This
	*   lasts until @p timeout seconds have passed (or forever
	*   if @p timeout is zero).
	*
	*   @note Do not call startTickle() twice with no intervening
	*         stopTickle().
	*/
	void startTickle(unsigned int timeout=0);

	/** Stop tickling the Handheld. This may block for some
	*   time (less than a second) to allow the tickle thread
	*   to finish.
	*/
	void stopTickle();

	/** Install a single file onto the device link. Full pathname
	*   @p f is used; in addition, if @p deleteFile is true remove
	*   the source file. Returns @c true if the install succeeded.
	*
	*   The default
	*/
	virtual bool installFile( const QString &f, const bool deleteFile ) = 0;

	/**
	* Notify the Pilot user that a conduit is running now.
	* On real devices, this prints out (on screen) which database
	* is now opened; useful for progress reporting.
	*
	* @return -1 on error
	* @note the default implementation returns 0
	*/
	virtual int openConduit();

	/** Returns a file handle for raw operations. Not recommended.
	*   On links with no physical device backing, returns -1.
	*
	*   @note the default implementation returns -1
	*/
	virtual int pilotSocket() const;

	/** Actually write an entry to the device link. The message
	*   @p s is guaranteed to be non-Null.
	*/
	virtual void addSyncLogEntryImpl( const QString &s ) = 0;

	/** User information structure. Should be filled in when a sync
	*   starts, so that conduits can use the information.
	*/
	KPilotUser  *fPilotUser;

	/** System information about the device. Filled in when the
	*   sync starts. Non-device links need to fake something.
	*/
	KPilotSysInfo *fPilotSysInfo;


private:
	bool fTickleDone;
	QThread *fTickleThread;

} ;

#endif
