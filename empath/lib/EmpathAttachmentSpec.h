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

#ifndef EMPATHATTACHMENTSPEC_H
#define EMPATHATTACHMENTSPEC_H

// Qt includes
#include <qstring.h>

// Local includes
#include "EmpathDefines.h"
#include "RMM_Enum.h"

class EmpathAttachmentSpec
{
	public:
		
		EmpathAttachmentSpec()
		{
			empathDebug("ctor");
		}

		EmpathAttachmentSpec(
				const QString & filename,
				const QString & description,
				RMM::CteType	encoding,
				const QString & type,
				const QString & subType,
				const QString & charset)
			:
				filename_		(filename),
				description_	(description),
				encoding_		(encoding),
				type_			(type),
				subType_		(subType),
				charset_		(charset)
		{
			empathDebug("ctor");
		}
				
		EmpathAttachmentSpec(const EmpathAttachmentSpec & a)
			:
				filename_		(a.filename_),
				description_	(a.description_),
				encoding_		(a.encoding_),
				type_			(a.type_),
				subType_		(a.subType_),
				charset_		(a.charset_)
		{
			empathDebug("copy ctor");
		}
	
			EmpathAttachmentSpec &
		operator = (const EmpathAttachmentSpec & a)
		{	
			empathDebug("operator =");

			if (this == &a) return *this;
			
			filename_		= a.filename_;
			description_	= a.description_;
			encoding_		= a.encoding_;
			type_			= a.type_;
			subType_		= a.subType_;
			charset_		= a.charset_;
			
			return *this;
		}
		
		virtual ~EmpathAttachmentSpec()
		{
			empathDebug("dtor");
		}
		
		QString filename()		const { return filename_;		}
		QString description()	const { return description_;	}
		RMM::CteType encoding()	const { return encoding_;		}
		QString type()			const { return type_;			}
		QString subType()		const { return subType_;		}
		QString charset()		const { return charset_;		}
		
		void setFilename	(const QString & s) { filename_		= s; }
		void setDescription	(const QString & s) { description_	= s; }
		void setEncoding	(RMM::CteType t)	{ encoding_		= t; }
		void setType		(const QString & s) { type_			= s; }
		void setSubType		(const QString & s) { subType_		= s; }
		void setCharset		(const QString & s) { charset_		= s; }
		
		const char * className() const { return "EmpathAttachmentSpec"; }
		
	private:
		
		QString filename_;
		QString description_;
		RMM::CteType encoding_;
		QString type_;
		QString subType_;
		QString charset_;
};

#endif

