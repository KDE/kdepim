/*
	Empath - Mailer for KDE

	Copyright (C) 1998 Rik Hemsley rik@kde.org

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma interface "EmpathFolder.h"
#endif

#ifndef EMPATHFOLDER_H
#define EMPATHFOLDER_H

// Qt includes
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "EmpathIndex.h"
#include "EmpathURL.h"
#include "RMM_MessageID.h"

class EmpathMailbox;
class EmpathIndexRecord;
class EmpathIndex;
class RMessage;
class EmpathIndexAllocator;

class EmpathFolder : public QObject
{
	Q_OBJECT

	public:

		EmpathFolder();

		EmpathFolder(const EmpathURL & url);
		
		EmpathIndexAllocator * indexAllocator() { return indexAllocator_; }

		virtual ~EmpathFolder();

		bool operator == (const EmpathFolder & other) const;

		EmpathFolder * parent() const;

		void setPixmap(const QString &);

		const QString &		pixmapName()	const { return pixmapName_;	}
		const EmpathURL &	url()			const { return url_;	}

		Q_UINT32	messageCount()			const
		{ return messageList_.count(); }

		Q_UINT32	unreadMessageCount()	const
		{ return messageList_.countUnread(); }

		Q_UINT32 id() const { return id_; }

		EmpathIndex & messageList() { return messageList_; }

		const EmpathIndexRecord *
			messageDescription(RMessageID & messageID) const;

		QString writeMessage(RMessage & message);
		bool removeMessage(const EmpathURL &);

		void update();

		/**
		 * @short Attempt to get the message
		 * This message is allocated with new. It is your responsibility to
		 * delete it.
		 */

		RMessage * message(const EmpathURL & url);

		bool mark(const EmpathURL &, RMM::MessageStatus);
		
		void dropIndex();
		
		void itemGone(const QString & s) { emit(itemLeft(s));		}
		void itemCome(const QString & s) { emit(itemArrived(s));	}
		
	protected slots:
		
		void s_update() { update(); }

	signals:

		void countUpdated(int, int);
		void itemArrived(const QString &);
		void itemLeft	(const QString &);

	private:

		EmpathFolder(const EmpathFolder &) : QObject() {}

		Q_UINT32 id_;

		Q_UINT32 messageCount_;
		Q_UINT32 unreadMessageCount_;

		QString pixmapName_;
		EmpathIndex	messageList_;

		EmpathURL url_;
		
		EmpathIndexAllocator * indexAllocator_;
};

#endif

