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

#ifndef RMM_FIELD_BODY_H
#define RMM_FIELD_BODY_H

#include <qstring.h>

#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

class RHeaderBody : public RMessageComponent {

	public:

		virtual ~RHeaderBody();

		virtual void parse() = 0L;
		virtual void assemble() = 0L;
		virtual void createDefault() = 0L;

		void set(const QString & s) { RMessageComponent::set(s); }
		const QString & asString() const { return RMessageComponent::asString(); }
		
		const RHeaderBody & operator = (const RHeaderBody &);

		const char * className() const { return "RHeaderBody"; }

	protected:

		RHeaderBody();
		RHeaderBody(const RHeaderBody & headerBody);
		RHeaderBody(const QString & s) : RMessageComponent(s) { }
};

#endif
