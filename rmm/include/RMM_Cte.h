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

class RCte : public RHeaderBody {

	public:

		RCte();
		RCte(const RCte & cte);
		RCte(const QCString & s) : RHeaderBody(s) { }
		const RCte & operator = (const RCte & cte);

		virtual ~RCte();

		void parse();
		void assemble();
		
		bool isValid() const;

		void createDefault();
		
		const QCString & mechanism() const { return mechanism_; }
		void setMechanism(const QCString & s) { mechanism_ = s; }
		
		const char * className() const { return "RCte"; }

	private:

		bool isValid_;
		QCString mechanism_;
};

#endif

