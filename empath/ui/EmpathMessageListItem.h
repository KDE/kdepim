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

#ifndef EMPATHMESSAGELISTITEM_H
#define EMPATHMESSAGELISTITEM_H

// Qt includes
#include <qstring.h>
#include <qlistview.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "RMM_MessageID.h"
#include "RMM_Mailbox.h"
#include "RMM_DateTime.h"
#include "RMM_Enum.h"

class EmpathMessageListWidget;

/**
 * @internal
 */
class EmpathMessageListItem : public QListViewItem
{
	public:
	
		EmpathMessageListItem(
			EmpathMessageListWidget * parent,
			EmpathIndexRecord & msgDesc);

		EmpathMessageListItem(
			EmpathMessageListItem * parent,
			EmpathIndexRecord & msgDesc);

		~EmpathMessageListItem();
		
		virtual void setup();

		QString key(int, bool) const;

		const QString &		id()		const	{ return id_;			}
		RMessageID &		messageID() 		{ return messageID_;	}
		RMessageID &		parentID()			{ return parentID_;		}
		const QString &		subject()	const	{ return subject_;		}
		RMailbox &			sender()			{ return sender_;		}
		RDateTime &			date()				{ return date_;			}
		RMM::MessageStatus	status()	const	{ return status_;		}
		Q_UINT32			size()		const	{ return size_;			}

		const char * className() const { return "EmpathMessageListItem"; }
		
	private:

		void _init();
		
		QString				id_;
		RMessageID			messageID_;
		RMessageID			parentID_;
		QString				subject_;
		RMailbox			sender_;
		RDateTime			date_;
		RMM::MessageStatus	status_;
		QString				niceDate_;
		Q_UINT32			size_;
		QString				dateStr_;
		QString				sizeStr_;
};

#endif

