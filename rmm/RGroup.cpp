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
# pragma implementation "RMM_Group.h"
#endif

#include <qstring.h>

#include <RMM_Group.h>
#include <RMM_Token.h>

using namespace RMM;

RGroup::RGroup()
{
	rmmDebug("ctor");
}

RGroup::RGroup(const RGroup & g)
	:	RAddress(g)
{
	rmmDebug("ctor");
}

RGroup::RGroup(const QCString & s)
	:	RAddress(s)
{
	rmmDebug("ctor");
}

RGroup::~RGroup()
{
	rmmDebug("dtor");
}

	RGroup &
RGroup::operator = (const RGroup & g)
{
	rmmDebug("operator =");
	
	if (this == &g) return *this;
	
	mailboxList_	= g.mailboxList_;
	name_			= g.name_;
	phrase_			= g.phrase_;
	
	RAddress::operator = (g);

	assembled_	= false;
	return *this;
}

	RGroup &
RGroup::operator = (const QCString & s)
{
	rmmDebug("operator =");
	
	RAddress::operator = (s);

	assembled_	= false;
	return *this;
}

	bool
RGroup::operator == (RGroup & g)
{
	parse();
	g.parse();

	return (
		mailboxList_	== g.mailboxList_	&&
		name_			== g.name_			&&
		phrase_			== g.phrase_);
}

	QDataStream &
operator >> (QDataStream & s, RGroup & group)
{
	s	>> group.name_
		>> group.phrase_;
	group.assembled_ = false;
	return s;
}
	
	QDataStream &
operator << (QDataStream & s, RGroup & group)
{
	s	<< group.name_
		<< group.phrase_;
	return s;
}

	QCString
RGroup::name()
{
	parse();
	return name_;
}

	QCString
RGroup::phrase()
{
	parse();
	return phrase_;
}

	void
RGroup::setName(const QCString & s)
{
	name_ = s;
	assembled_ = false;
}

	void
RGroup::setPhrase(const QCString & s)
{
	phrase_ = s;
	assembled_ = false;
}

	RMailboxList &
RGroup::mailboxList()
{
	parse();
	return mailboxList_;
}

	void
RGroup::_parse()
{
}

	void
RGroup::_assemble()
{
}

	void
RGroup::createDefault()
{
	rmmDebug("createDefault() called");
	name_ = "unnamed";
	assembled_ = false;
}

