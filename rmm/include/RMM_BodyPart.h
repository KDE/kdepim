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

#ifndef RMM_BODY_PART_H
#define RMM_BODY_PART_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Entity.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>

class RBodyPart : public REntity {
	
	public:
	
		RBodyPart();
		RBodyPart(const RBodyPart & part);
		RBodyPart(const QCString & s) : REntity(s) { }

		const RBodyPart & operator = (const RBodyPart & part);

		virtual ~RBodyPart();
		void parse();
		void assemble();
		void createDefault();
		
		RMM::MimeType mimeType() const;
		RMM::MimeSubType mimeSubType() const;
		
		void setMimeType(RMM::MimeType t);
		void setMimeType(const QCString & s);
		void setMimeSubType(RMM::MimeSubType st);
		void setMimeSubType(const QCString & s);

		const QCString & description() const;
		RMM::DispType disposition() const;
		
		void setDescription(const QCString & s);
		void setDisposition(RMM::DispType d);
		
		RMM::CteType encoding() const;
		void setEncoding(RMM::CteType e);
		
		const char * className() const { return "RBodyPart"; }

	protected:

		QByteArray body_;
		RMM::CteType encoding_;
		RMM::MimeType mimeType_;
		RMM::MimeSubType mimeSubType_;
		QCString contentDescription_;
		RMM::DispType disposition_;

};

typedef QList<RBodyPart>			RBodyPartList;
typedef QListIterator<RBodyPart>	RBodyPartListIterator;

#endif

