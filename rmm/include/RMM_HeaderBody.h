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

#ifndef RMM_FIELD_BODY_H
#define RMM_FIELD_BODY_H

#include <qstring.h>

#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

/**
 * Base class for RAddress, RContentType, RMailboxList etc.
 */
class RHeaderBody : public RMessageComponent {

	public:

		virtual ~RHeaderBody();

		virtual void parse() 
			{ rmmDebug("WARNING base class parse() called"); }

		virtual void assemble()
			{ rmmDebug("WARNING base class assemble() called"); }

		virtual void createDefault()
			{ rmmDebug("WARNING base class createDefault() called"); }
			
		virtual const RHeaderBody & operator = (const RHeaderBody & h);

		void set(const QCString & s) { RMessageComponent::set(s); }
		const QCString & asString() const { return RMessageComponent::asString(); }
		
		const char * className() const { return "RHeaderBody"; }

	protected:

		RHeaderBody();
		RHeaderBody(const RHeaderBody & headerBody);
		RHeaderBody(const QCString & s) : RMessageComponent(s) { }
		
	friend class RHeader; // FIXME: Hack. Remove.
};

#endif
