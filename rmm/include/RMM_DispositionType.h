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

#ifndef RMM_RDISPOSITIONTYPE_H
#define RMM_RDISPOSITIONTYPE_H

#include <qstring.h>
#include <RMM_Enum.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>
#include <RMM_Parameter.h>
#include <RMM_ParameterList.h>

class RDispositionType : public RHeaderBody {

	public:

		RDispositionType();
		RDispositionType(const RDispositionType & t);
		RDispositionType(const QCString & s) : RHeaderBody(s) { }
		RDispositionType & operator = (const RDispositionType & t);

		virtual ~RDispositionType();

		void parse();
		void assemble();

		void set(RMM::DispType);
		void set(const QCString & s) { RHeaderBody::set(s); }

		QCString filename();
		void setFilename(const QCString &);
		void addParameter(RParameter & p);
		RParameterList & parameterList();
		RMM::DispType type();
		
		void createDefault();
		
		const char * className() const { return "RDispositionType"; }

	private:

		RParameterList	parameterList_;
		RMM::DispType	dispType_;
		QCString		filename_;
};

#endif //RDISPOSITIONTYPE_H
