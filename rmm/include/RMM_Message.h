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

#ifndef RMM_MESSAGE_H
#define RMM_MESSAGE_H

#include <RMM_Entity.h>
#include <RMM_Envelope.h>
#include <RMM_BodyPart.h>
#include <RMM_Body.h>
#include <RMM_Defines.h>

class RMessage : public REntity {

	public:

		enum MessageType {
			BasicMessage,
			MimeMessage
		};
		
		RMessage();
		RMessage(const RMessage &);
		RMessage(const QCString & s) : REntity(s) {
			rmmDebug("ctor");
			rmmDebug("Data:\n" + strRep_);
		}

		virtual ~RMessage();

		unsigned long int id() const { return id_; }

		const RMessage & operator = (const RMessage & message);
		
		bool operator == (const RMessage & m) const
		{ return id_ == m.id_; }
		
		friend QDataStream & operator << (QDataStream & str, const RMessage & m)
		{ str << m.asString(); return str; }

		QCString recipientListAsPlainString();

		REnvelope	& envelope()			{ return envelope_; }
		RBody		& body() 				{ return body_; }

		Q_UINT32 size() const				{ return strRep_.length(); }
		
		void set(const QCString & s)			{ REntity::set(s); }
		const QCString & asString() const	{ return REntity::asString(); }
		
		int			numberOfParts() const;
		void		addPart(RBodyPart * bp);
		void		removePart(RBodyPart * part);
		
		RBodyPart	* part(int index);

		MessageType	type() const;
		
//		void addAttachment(const EmpathAttachmentSpec & att);
//		void addAttachmentList(const QList<EmpathAttachmentSpec> & attList);
		bool hasParentMessageID() const;

		void setStatus(MessageStatus status)		{ status_ = status; }
		MessageStatus status() const				{ return status_; }
		void setFolder(const QCString & folderName)	{ folder_ = folderName.data();}
		const QCString & folder() const				{ return folder_; }

		const char * className() const				{ return "RMessage"; }

		void parse();
		void assemble();
		void createDefault();
		
	protected:
		
		void				_update();
		MessageType			type_;

		REnvelope			envelope_;
		RBody				body_;
		
		MessageStatus		status_;
		QCString				folder_;	
		unsigned long int	id_;
};

#endif

