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

#ifndef RMM_CONTENTTYPE_H
#define RMM_CONTENTTYPE_H

#include <qstring.h>

#include <RMM_HeaderBody.h>
#include <RMM_ParameterList.h>

/**
 * An RContentType has a mime type, a mime subtype, and a parameter list.
 */
class RContentType : public RHeaderBody {

	public:

		RContentType();
		RContentType(const RContentType & ct);
		RContentType(const QCString & s) : RHeaderBody(s) { }
		const RContentType & operator = (const RContentType & ct);
		
//		friend QDataStream & operator >> (
//			QDataStream & s, RContentType & ct);
		
//		friend QDataStream & operator << (
//			QDataStream & s, const RContentType & ct);

		virtual ~RContentType();

		void parse();
		void assemble();

		bool isValid() const;

		void createDefault();

		void setType(const QCString & type)				{ type_ = type; }
		void setSubType(const QCString & subType)		{ subType_ = subType; }
		void setParameterList(const RParameterList & pl){ parameterList_ = pl; }
		const QCString & type() const					{ return type_; }
		const QCString & subType() const					{ return subType_; }
		const RParameterList & parameterList() const	{ return parameterList_; }
		
		const char * className() const { return "RContentType"; }

	private:

		QCString type_;
		QCString subType_;
		RParameterList parameterList_;
		bool isValid_;
};

#endif //RMM_CONTENTTYPE_H
