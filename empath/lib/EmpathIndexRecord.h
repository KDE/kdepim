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
			
		EmpathIndexRecord(
				const QString & id,
				const QString & subject,
				const RMailbox & sender,
				const RDateTime & date,
				int status,
				Q_UINT32 size,
				const RMessageID & messageID,
				const RMessageID & parentMessageID);

		~EmpathIndexRecord();

		const char * className() { return "EmpathIndexRecord"; }

		const QString & 	id()						const;
		const QString &		subject()					const;
		const RMailbox &	sender()					const;
		const RDateTime &	date()						const;
		QString				niceDate(bool twelveHour)	const;
		MessageStatus		status()					const;
		Q_UINT32			size()						const;
		bool				hasParent()					const;
		const RMessageID &	messageID()					const;
		const RMessageID &	parentID()					const;

		void setStatus(int status);
		
	private:
		
		// Order dependency
		QString				id_;
		QString 			subject_;
		RMailbox 			sender_;
		RDateTime			date_;
		int					status_;
		Q_UINT32			size_;
		RMessageID			messageId_;
		RMessageID			parentMessageId_;
		bool				hasParent_;
};

#endif

