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

#ifndef RMM_MEDIATYPE_H
#define RMM_MEDIATYPE_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_HeaderBody.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>

class RMimeType : public RHeaderBody {

	public:

		RMimeType();
		RMimeType(const RMimeType & mt);
		RMimeType(const QCString & s) : RHeaderBody(s) { }

		virtual ~RMimeType();

		const RMimeType & operator = (const RMimeType &);

		void parse();
		void assemble();

		const QCString & boundary() const;
		const QCString & name() const;

		RMM::MimeType type() const;
		RMM::MimeSubType subType() const;

		void setType(RMM::MimeType);
		void setType(const QCString &);
		void setSubType(RMM::MimeSubType);
		void setSubType(const QCString &);

		void setBoundary(const QCString & boundary);
		void setName(const QCString & name);

		const char * className() const { return "RMimeType"; }

	private:

		QCString boundary_;
		QCString name_;
		
		RMM::MimeType type_;
		RMM::MimeSubType subType_;
		
		RParameterList	parameterList_;

};

#endif
