#ifndef _KPILOT_KPILOTDEVICELINKPRIVATE_H
#define _KPILOT_KPILOTDEVICELINKPRIVATE_H
/*
**
** Copyright (C) 2007 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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

#include <errno.h>

#include <qstringlist.h>
#include <qthread.h>

#include "kpilotdevicelink.h"
#include "options.h"

class QTimer;
class QSocketNotifier;

// singleton helper class
class DeviceMap
{
public:
	static DeviceMap *self()
	{
		if (!mThis)
			mThis = new DeviceMap();
		return mThis;
	}

	bool canBind(const QString &device)
	{
		FUNCTIONSETUPL(5);
		DEBUGKPILOT << fname << ": device: ["
			<< device << "]" << endl;

		showList();
		return !mBoundDevices.contains(device);
	}

	void bindDevice(const QString &device)
	{
		FUNCTIONSETUPL(5);
		DEBUGKPILOT << fname << ": device: ["
			<< device << "]" << endl;

		mBoundDevices.append(device);
		showList();
	}

	void unbindDevice(const QString &device)
	{
		FUNCTIONSETUPL(5);
		DEBUGKPILOT << fname << ": device: ["
			<< device << "]" << endl;

		mBoundDevices.remove(device);
		showList();
	}

protected:
	DeviceMap()
	{
		mBoundDevices.clear();
	}
	~DeviceMap()
	{
	}

	QStringList mBoundDevices;
	static DeviceMap *mThis;

private:
	void showList() const
	{
		FUNCTIONSETUPL(5);

		if ( !(mBoundDevices.count() > 0))
			return;

		DEBUGKPILOT << fname << ": Bound devices: ["
			<< ((mBoundDevices.count() > 0) ?
					mBoundDevices.join(CSL1(", ")) : CSL1("<none>"))
			<< "]" << endl;
	}
};

class Messages
{
public:
	Messages(KPilotDeviceLink *parent) :
		fDeviceLink(parent)
	{
		reset();
	}

	void reset()
	{
		messages = 0;
		messagesMask = ~messageIsError; // Never block errors
	}

	void block(unsigned int m, bool force=false)
	{
		if (force)
		{
			// Force blocking this message, even if it's an error msg.
			messages |= m;
		}
		else
		{
			messages |= (m & messagesMask);
		}
	}

	/**
	 * Some messages are only printed once and are suppressed
	 * after that. These are indicated by flag bits in
	 * messages. The following enum is a bitfield.
	 */
	enum
	{
		OpenMessage=1, ///< Trying to open device ..
		OpenFailMessage=2 ///< Failed to open device ...
	};
	int messages;
	int messagesMask;
	static const int messageIsError = 0;

	/** Determines whether message @p s which has an id of @p msgid (one of
	 *  the enum values mentioned above) should be printed, which is only if that
	 *  message has not been suppressed through messagesMask.
	 *  If return is true, this method also adds it to the messagesMask.
	 */
	bool shouldPrint(int msgid)
	{
		if (!(messages & msgid))
		{
			block(msgid);
			return true;
		}
		else
		{
			return false;
		}
	}

protected:
	KPilotDeviceLink *fDeviceLink;
};

class DeviceCommEvent : public QEvent
{
public:
	DeviceCommEvent(DeviceCustomEvents type, QString msg = QString::null,
			int progress = 0) :
		QEvent( (QEvent::Type)type ), fMessage(msg), fProgress(progress),
				fPilotSocket(-1)
	{
	}
	QString message() const
	{
		return fMessage;
	}
	int progress()
	{
		return fProgress;
	}

	inline void setCurrentSocket(int i)
	{
		fPilotSocket = i;
	}

	inline int currentSocket()
	{
		return fPilotSocket;
	}
private:
	QString fMessage;
	int fProgress;
	/**
	 * Pilot-link library handles for the device once it's opened.
	 */
	int fPilotSocket;
};

/** Class that handles all device communications.  We do this
 in a different thread so that we do not block the main Qt
 Event thread (similar to Swing's AWT event dispatch thread).
 */

class DeviceCommThread : public QObject, public QThread
{
friend class KPilotDeviceLink;

Q_OBJECT

public:
	DeviceCommThread(KPilotDeviceLink *d);
	virtual ~DeviceCommThread();

	virtual void run();

	void setDone(bool b)
	{
		FUNCTIONSETUP;
		fDone = b;
	}

protected:

	void close();
	
	void reset();

	/**
	 * Does the low-level opening of the device and handles the
	 * pilot-link library initialisation.
	 */
	bool open(const QString &device = QString::null);

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
	
	/**
	* This slot fires whenever we've been trying to establish a hotsync with
	* the device for longer than a given amount of time.  When this slot is
	* fired, we will tear down the communications process and start over again.
	*/
	void workaroundUSB();

private:
	volatile bool fDone;

	KPilotDeviceLink *fHandle;
	inline KPilotDeviceLink *link()
	{
		if (fHandle)
		{
			return fHandle;
		}
		else
		{
			FUNCTIONSETUP;
			WARNINGKPILOT << "Link asked for, but either I'm "
				<< "done or I don't have a valid handle.  "
				<< "Shutting down comm thread." << endl;
			QThread::exit();
			return 0;
		}
	}

	/**
	 * Timers and Notifiers for detecting activity on the device.
	 */
	QTimer *fOpenTimer;
	QSocketNotifier *fSocketNotifier;
	bool fSocketNotifierActive;

	/** Timer used to check for a badly-connected Z31/72 */
	QTimer *fWorkaroundUSBTimer;
	
	/**
	 * Pilot-link library handles for the device once it's opened.
	 */
	int fPilotSocket;
	int fTempSocket;

	inline QString errorMessage(int e)
	{
		switch (e)
		{
		case ENOENT:
			return i18n(" The port does not exist.");
			break;
		case ENODEV:
			return i18n(" There is no such device.");
			break;
		case EPERM:
			return i18n(" You do not have permission to open the "
				"Pilot device.");
			break;
		default:
			return i18n(" Check Pilot path and permissions.");
		}
	}
	
	/**
	* Handle cases where we can't accept or open the device,
	* and data remains available on the pilot socket.
	*/
	int fAcceptedCount;

};


#endif

