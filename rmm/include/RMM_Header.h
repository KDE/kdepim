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

#ifndef R_HEADER_H
#define R_HEADER_H

#include <qlist.h>

#include <RMM_Enum.h>
#include <RMM_HeaderBody.h>
#include <RMM_Defines.h>

class RHeader : public RMessageComponent
{
	public:
		
		RHeader();
		RHeader(const QCString & name, RHeaderBody * b);
		RHeader(const QCString & s) : RMessageComponent(s), headerBody_(0) { }
		RHeader(HeaderType t, RHeaderBody * b);
		const RHeader & operator = (const RHeader & h);

		virtual ~RHeader();

		void parse();
		void assemble();
		void createDefault();
		
		const QCString & headerName() const;
		HeaderType headerType() const;
		RHeaderBody * headerBody() const;

		void setName(const QCString & name);
		void setType(HeaderType t);
		void setBody(RHeaderBody * b);

		const char * className() const { return "RHeader"; }

	private:
		
		QCString headerName_;
		HeaderType headerType_;
		RHeaderBody * headerBody_;
};

typedef QListIterator<RHeader> RHeaderListIterator;
typedef QList<RHeader> RHeaderList;

#endif

