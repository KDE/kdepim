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
# pragma interface "EmpathIndexRecord.h"
#endif

#ifndef EMPATHMESSAGEDESCRIPTION_H
#define EMPATHMESSAGEDESCRIPTION_H

// Qt includes
#include <qstring.h>
#include <qlist.h>

// Local includes
#include "EmpathIndexAllocator.h"
#include "EmpathDefines.h"
#include <RMM_Enum.h>
#include <RMM_Message.h>
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_DateTime.h>


class EmpathIndexRecord
{
	public:
		
		static void * operator new(size_t _size, EmpathIndexAllocator *a)
		{ return a->allocate(_size); }
		
		static void operator delete(void *) { /* nothing */ }
		
		EmpathIndexRecord();
			
		EmpathIndexRecord(const QString & id, RMM::RMessage &);
		EmpathIndexRecord(const EmpathIndexRecord &);

		EmpathIndexRecord(
				const QString &		id,
				const QString &		subject,
				RMM::RMailbox &		sender,
				RMM::RDateTime &	date,
				RMM::MessageStatus	status,
				Q_UINT32			size,
				RMM::RMessageID &	messageID,
				RMM::RMessageID &	parentMessageID);

		~EmpathIndexRecord();
		
		friend QDataStream &
			operator << (QDataStream &, EmpathIndexRecord &);
		
		friend QDataStream &
			operator >> (QDataStream &, EmpathIndexRecord &);

		const char * className() const { return "EmpathIndexRecord"; }

		const QString & 	id()		const	{ return id_;				}
		const QString &		subject()	const	{ return subject_;			}
		RMM::RMailbox &		sender()			{ return sender_;			}
		RMM::RDateTime &	date()				{ return date_;				}
		RMM::MessageStatus	status()	const	{ return status_;			}
		Q_UINT32			size()		const	{ return size_;				}
		RMM::RMessageID &	messageID()			{ return messageId_;		}
		RMM::RMessageID &	parentID()			{ return parentMessageId_;	}
		
		bool				hasParent();
		QString				niceDate(bool twelveHour);

		void setStatus(RMM::MessageStatus s);
		
		void tag(bool b){ tagged_ = b; }
		bool isTagged()	{ return tagged_; }
		
	private:
		
		// Order dependency
		QString				id_;
		QString 			subject_;
		RMM::RMailbox 		sender_;
		RMM::RDateTime		date_;
		RMM::MessageStatus	status_;
		Q_UINT32			size_;
		RMM::RMessageID		messageId_;
		RMM::RMessageID		parentMessageId_;
		
		bool				tagged_;
};

class EmpathIndexRecordList : public QList<EmpathIndexRecord>
{
	public:
		EmpathIndexRecordList() : QList<EmpathIndexRecord>() {}
		virtual ~EmpathIndexRecordList() {}
		
	protected:
		virtual int compareItems(void * i1, void * i2)
		{
			return
				((EmpathIndexRecord *)i1)->date().qdt() >
				((EmpathIndexRecord *)i2)->date().qdt() ? 1 : -1;
		}
};

#endif

