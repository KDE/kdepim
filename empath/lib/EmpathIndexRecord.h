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

#ifndef EMPATHMESSAGEDESCRIPTION_H
#define EMPATHMESSAGEDESCRIPTION_H

// Qt includes
#include <qstring.h>

// KDE includes
#include <RMM_Enum.h>
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_DateTime.h>

// Local includes
#include "EmpathDefines.h"

class RMessage;
	
class EmpathIndexRecord
{
	public:
		
		EmpathIndexRecord();
			
		EmpathIndexRecord(const QString & id, RMessage &);

		EmpathIndexRecord(
				const QString &		id,
				const QString &		subject,
				RMailbox &			sender,
				RDateTime &			date,
				RMM::MessageStatus	status,
				Q_UINT32			size,
				RMessageID &		messageID,
				RMessageID &		parentMessageID);

		~EmpathIndexRecord();
		
		friend QDataStream &
			operator << (QDataStream &, EmpathIndexRecord &);
		
		friend QDataStream &
			operator >> (QDataStream &, EmpathIndexRecord &);

		const char * className() const { return "EmpathIndexRecord"; }

		const QString & 	id()		const	{ return id_;				}
		const QString &		subject()	const	{ return subject_;			}
		RMailbox &			sender()			{ return sender_;			}
		RDateTime &			date()				{ return date_;				}
		RMM::MessageStatus	status()	const	{ return status_;			}
		Q_UINT32			size()		const	{ return size_;				}
		RMessageID &		messageID()			{ return messageId_;		}
		RMessageID &		parentID()			{ return parentMessageId_;	}

		bool				hasParent();
		QString				niceDate(bool twelveHour)	const;

		void setStatus(RMM::MessageStatus s) { status_ = s; }
		
		void tag()		{ tagged_ = true; }
		bool isTagged()	{ return tagged_; }
		
	private:
		
		// Order dependency
		QString				id_;
		QString 			subject_;
		RMailbox 			sender_;
		RDateTime			date_;
		RMM::MessageStatus	status_;
		Q_UINT32			size_;
		RMessageID			messageId_;
		RMessageID			parentMessageId_;
		
		bool				tagged_;
};

#endif

