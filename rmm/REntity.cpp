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

#include <qstring.h>

#include <RMM_Envelope.h>
#include <RMM_Entity.h>
#include <RMM_Body.h>
#include <RMM_MimeType.h>

REntity::REntity()
{
	rmmDebug("ctor");
}

REntity::REntity(const REntity & e)
	:	RMessageComponent()
{
	rmmDebug("ctor");
}

REntity::~REntity()
{
	rmmDebug("dtor");
}

	const REntity &
REntity::operator = (const REntity & e)
{
	rmmDebug("operator =");
    if (this == &e) return *this; // Don't do a = a.
	RMessageComponent::operator = (e);
	return *this;
}

