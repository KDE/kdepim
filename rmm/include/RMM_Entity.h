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

#ifndef DW_ENTITY_H
#define DW_ENTITY_H

#include <qstring.h>
#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

class REntity : public RMessageComponent
{
	public:

		REntity();
		REntity(const REntity & entity);
		REntity(const QString & s) : RMessageComponent(s) { }

		virtual ~REntity();

		const REntity & operator = (const REntity & entity);

		virtual void parse() = 0L;
		virtual void assemble() = 0L;
		virtual void createDefault() = 0L;
		
		void set(const QString & s) { strRep_ = s.data(); }
		const QString & asString() const { return strRep_; }
		
		const char * className() const { return "REntity"; }

	protected:

};

#endif
