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
#include <RMM_Enum.h>

class RMessage : public REntity {

	public:

		enum MessageType {
			BasicMessage,
			MimeMessage
		};
		
		RMessage();
		RMessage(const RMessage &);
		RMessage(const QCString & s);

		virtual ~RMessage();

		unsigned long int id() const { return id_; }

		RMessage & operator = (const RMessage & message);
		
		bool operator == (RMessage & m)
		{ return id_ == m.id_; }
		
		friend QDataStream & operator << (QDataStream & str, RMessage & m)
		{ str << m.asString(); return str; }

		QCString recipientListAsPlainString();

		REnvelope	& envelope();
		RBody		& body();

		Q_UINT32 size();
		
		void set(const QCString & s)		{ REntity::set(s); }
		QCString asString()			{ return REntity::asString(); }
		
		int			numberOfParts();
		void		addPart(RBodyPart * bp);
		void		removePart(RBodyPart * part);
		
		RBodyPart	part(int index);

		MessageType	type();
		
		bool hasParentMessageID();

		void setStatus(RMM::MessageStatus status);
		RMM::MessageStatus status();

		const char * className();

		void parse();
		void assemble();
		void createDefault();
		
	protected:
		
		void				_update();
		MessageType			type_;

		REnvelope			envelope_;
		RBody				body_;
		
		RMM::MessageStatus	status_;
		QCString			folder_;	
		unsigned long int	id_;
};

#endif

