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

#ifndef RMM_ENVELOPE_H
#define RMM_ENVELOPE_H

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>

#include <RMM_Entity.h>
#include <RMM_MessageID.h>
#include <RMM_Mailbox.h>
#include <RMM_MailboxList.h>
#include <RMM_MimeType.h>
#include <RMM_Mechanism.h>
#include <RMM_DispositionType.h>
#include <RMM_DateTime.h>
#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_Text.h>
#include <RMM_Enum.h>
#include <RMM_Header.h>
#include <RMM_Defines.h>
#include <RMM_MessageComponent.h>

class REnvelope : public RMessageComponent
{
	public:

		REnvelope();
		REnvelope(const REnvelope & envelope);
		REnvelope(const QCString & s) : RMessageComponent(s) { }

		virtual ~REnvelope();
		const REnvelope & operator = (const REnvelope & envelope);

		void parse();
		void assemble();

		bool has(RMM::HeaderType t) const;
		bool has(const QCString & headerName) const;

		void set(const QCString & s)
		{ RMessageComponent::set(s); }
		
		const QCString & asString() const
		{ return RMessageComponent::asString(); }
		
		void set(RMM::HeaderType t, const QCString & s);
		void _createDefault(RMM::HeaderType t);
		void createDefault();
		
		const RMailbox & firstSender();
		RMessageID parentMessageId();

		RText &				get(const QCString & headerName);

		template <class T> T get(RMM::HeaderType h, T t);

		RText &				approved();
		RAddressList &		bcc();
		RMailboxList &		cc();
		RText &				comments();
		RText &				contentDescription();
		RDispositionType &	contentDisposition();
		RMessageID &		contentID();
		RText &				contentMD5();
		RText &				contentType();
		RText &				control();
		RText &				contentTransferEncoding();
		RDateTime &			date();
		RText &				distribution();
		RText &				encrypted();
		RDateTime &			expires();
		RText &				followupTo();
		RMailboxList &		from();
		RText &				inReplyTo();
		RText &				keywords();
		RText &				lines();
		RMessageID &		messageID();
		RText &				mimeVersion();
		RText &				newsgroups();
		RText &				organization();
		RText &				path();
		RText &				received();
		RText &				references();
		RAddressList &		replyTo();
		RAddressList &		resentBcc();
		RAddressList &		resentCc();
		RDateTime &			resentDate();
		RMailboxList &		resentFrom();
		RMessageID &		resentMessageID();
		RAddressList &		resentReplyTo();
		RMailbox &			resentSender();
		RAddressList &		resentTo();
		RText &				returnPath();
		RMailbox &			sender();
		RText &				subject();
		RText &				summary();
		RAddressList &		to();
		RText &				xref();
		
		const char * className() const { return "REnvelope"; }

	private:

		RHeaderList headerList_;
};

#endif // RMM_ENVELOPE_H
