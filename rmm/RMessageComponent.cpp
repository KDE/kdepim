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

#include <stdlib.h>

#include <RMM_MessageComponent.h>

RMessageComponent::RMessageComponent()
	:	parsed_		(false),
		assembled_	(false)
{
	rmmDebug("ctor");
}

RMessageComponent::RMessageComponent(const RMessageComponent & mc)
	:	strRep_(mc.strRep_),
		parsed_(mc.parsed_),
		assembled_(mc.assembled_)
{
	rmmDebug("ctor");
}

RMessageComponent::~RMessageComponent()
{
	rmmDebug("dtor");
}

	RMessageComponent &
RMessageComponent::operator = (const RMessageComponent & m)
{
	rmmDebug("operator =");
	parsed_		= m.parsed_;
	assembled_	= m.assembled_;
	strRep_ = m.strRep_;
	return *this;
}

	RMessageComponent &
RMessageComponent::operator = (const QCString & s)
{
	strRep_		= s;
	parsed_		= false;
}

	
RMessageComponent::RMessageComponent(const QCString & s)
	:	strRep_(s),
		parsed_(false)
{
}
