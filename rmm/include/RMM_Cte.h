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

#ifndef RMM_CTE_H
#define RMM_CTE_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Defines.h>
#include <RMM_Parameter.h>
#include <RMM_HeaderBody.h>

/**
 * An RCte holds a Content-Transfer-Encoding header body. It contains a
 * mechanism. This is likely to be "7bit, "quoted-printable", "base64", "8bit",
 * "binary" or an 'x-token'. An x-token is an extension token and is prefixed
 * by 'X-'.
 */
class RCte : public RHeaderBody {

	public:

		RCte();
		RCte(const RCte & cte);
		RCte(const QCString & s) : RHeaderBody(s) { }
		RCte & operator = (const RCte & cte);

		virtual ~RCte();

		void parse();
		void assemble();
		
		void createDefault();
		
		const QCString & mechanism();
		void setMechanism(const QCString &);
		
		const char * className() const { return "RCte"; }

	private:

		QCString mechanism_;
};

#endif

