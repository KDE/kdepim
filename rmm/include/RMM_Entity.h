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

#ifdef __GNUG__
# pragma interface "RMM_Entity.h"
#endif

#ifndef RMM_ENTITY_H
#define RMM_ENTITY_H

#include <qstring.h>
#include <RMM_MessageComponent.h>
#include <RMM_Defines.h>

namespace RMM {

/**
 * @short An REntity is the base class of an RBodyPart and an RMessage.
 * An REntity is the base class of an RBodyPart and an RMessage. Note that the
 * RFC822 specification is recursive. That means that an RBodyPart can also be
 * an RMessage, which then in turn contains an RBodyPart !
 */
class REntity : public RMessageComponent
{
	public:

		REntity() : RMessageComponent() {}
		REntity(const REntity & e) : RMessageComponent(e) {}
		REntity(const QCString & s) : RMessageComponent(s) {}

		virtual ~REntity();

		virtual void parse() = 0L;
		virtual void assemble() = 0L;
		virtual void createDefault() = 0L;
		
		virtual const char * className() const { return "REntity"; }
	
	protected:
		
		RMessageComponent::strRep_;
		RMessageComponent::parsed_;
		RMessageComponent::assembled_;
};

}

#endif
