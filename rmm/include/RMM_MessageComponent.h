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

#ifndef RMM_MESSAGE_COMPONENT_H
#define RMM_MESSAGE_COMPONENT_H

#include <sys/types.h>

#include <qstring.h>
#include <RMM_Defines.h>

class RMessageComponent {

	public:
		

		virtual ~RMessageComponent();

		RMessageComponent & operator = (const QString & s)
		{ strRep_ = s; }
		
		RMessageComponent & operator = (const RMessageComponent & m);

		virtual void parse() = 0L;

		virtual void assemble() = 0L;
		
		virtual void createDefault() = 0L;

		void set(const QString & s) { strRep_ = s.data(); }

		const QString & asString() const { return strRep_; }

		RMessageComponent * parent();

		const char * className() const { return "RMessageComponent"; }

		bool isDirty() const { return dirty_; }
		void touch() { dirty_ = true; }
		
	protected:

		RMessageComponent();
		RMessageComponent(const RMessageComponent & component);
		RMessageComponent(const QString & s) : strRep_(s.data()) { }

		QString strRep_;
		bool 				isModified_;
		RMessageComponent	* parent_;
		bool				dirty_;
};

#endif
