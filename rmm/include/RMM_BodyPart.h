/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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
		
		MimeType mimeType() const;
		MimeSubType mimeSubType() const;
		
		void setMimeType(MimeType t);
		void setMimeType(const QCString & s);
		void setMimeSubType(MimeSubType st);
		void setMimeSubType(const QCString & s);

		const QCString & description() const;
		DispType disposition() const;
		
		void setDescription(const QCString & s);
		void setDisposition(DispType d);
		
		CteType encoding() const;
		void setEncoding(CteType e);
		
		const char * className() const { return "RBodyPart"; }
    
	protected:
    
		QByteArray body_;
		CteType encoding_;
		MimeType mimeType_;
		MimeSubType mimeSubType_;
		QCString contentDescription_;
		DispType disposition_;

};

#endif

