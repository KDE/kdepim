/*
	libvcard - vCard parsing library for vCard version 3.0

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
# pragma implementation "VCard.h"
#endif

#include <iostream>

#include <qcstring.h>
#include <qstrlist.h>

#include <Entity.h>
#include <VCard.h>
#include <ContentLine.h>
#include <RToken.h>

#include <VCardDefines.h>

namespace VCARD
{

VCard::VCard()
	:	Entity()
{
}

VCard::VCard(const VCard & x)
	:	Entity(x),
		group_(x.group_),
		contentLineList_(x.contentLineList_)
{
}

VCard::VCard(const QCString & s)
	:	Entity(s)
{
}

	VCard &
VCard::operator = (VCard & x)
{
	if (*this == x) return *this;
	
	group_				= x.group();
	contentLineList_	= x.contentLineList_;

	Entity::operator = (x);
	return *this;
}

	VCard &
VCard::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
VCard::operator == (VCard & x)
{
	x.parse();
	return false;
}

VCard::~VCard()
{
}

	void
VCard::_parse()
{
	vDebug("parse() called");
	QStrList l;
	
	RTokenise(strRep_, "\r\n", l);
	
	if (l.count() < 3) { // Invalid VCARD !
		vDebug("Invalid vcard");
		return;
	}
	
	// Get the first line
	QCString beginLine = l.at(0);
	
	// Remove extra blank lines
	while (QCString(l.last()).isEmpty())
		l.remove(l.last());
	
	// Now we know this is the last line
	QCString endLine = l.last();
	
	// Trash the first and last lines as we have seen them.
	l.remove(0u);
	l.remove(l.last());

	///////////////////////////////////////////////////////////////
	// FIRST LINE
	
	int split = beginLine.find(':');
	
	if (split == -1) { // invalid, no BEGIN
		vDebug("No split");
		return;
	}
	
	QCString firstPart(beginLine.left(split));
	QCString valuePart(beginLine.mid(split + 1));
	
	split = firstPart.find('.');
	
	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}
	
	if (stricmp(firstPart, "BEGIN") != 0) { // No BEGIN !
		vDebug("No BEGIN");
		return;
	}
	
	if (stricmp(valuePart, "VCARD") != 0) { // Not a vcard !
		vDebug("No VCARD");
		return;
	}
	
	///////////////////////////////////////////////////////////////
	// CONTENT LINES
	// 
	vDebug("Content lines");
	
	QStrListIterator it(l);
	
	for (; it.current(); ++it) {
		
		vDebug("New contentline using \"" + QCString(it.current()) + "\"");
		ContentLine * cl = new ContentLine(it.current());

		cl->parse();
		
		contentLineList_.append(cl);
	}
	
	///////////////////////////////////////////////////////////////
	// LAST LINE
	
	split = endLine.find(':');
	
	if (split == -1) // invalid, no END
		return;
	
	firstPart = endLine.left(split);
	valuePart = endLine.right(firstPart.length() - split - 1);
	
	split = firstPart.find('.');
	
	if (split != -1) {
		group_		= firstPart.left(split);
		firstPart	= firstPart.right(firstPart.length() - split - 1);
	}
	
	if (stricmp(firstPart, "END") != 0) // No END !
		return;
	
	if (stricmp(valuePart, "VCARD") != 0) // Not a vcard !
		return;
}

	void
VCard::_assemble()
{
	vDebug("Assembling vcard");
	strRep_ = "BEGIN:VCARD\r\n";
//	strRep_ += "VERSION:3.0\r\n";
	
	QListIterator<ContentLine> it(contentLineList_);
	
	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + "\r\n";
	
	strRep_ += "END:VCARD\r\n";
}

	bool
VCard::has(EntityType t)
{
	parse();
	return contentLine(t) == 0 ? false : true; 
}

	bool
VCard::has(const QCString & s)
{
	parse();
	return contentLine(s) == 0 ? false : true; 
}

	void
VCard::add(const ContentLine & cl)
{
	parse();
	ContentLine * c = new ContentLine(cl);
	contentLineList_.append(c);
}

	void
VCard::add(const QCString & s)
{
	parse();
	ContentLine * c = new ContentLine(s);
	contentLineList_.append(c);
}

	ContentLine *
VCard::contentLine(EntityType t)
{
	parse();
	QListIterator<ContentLine> it(contentLineList_);
	
	for (; it.current(); ++it)
		if (it.current()->entityType() == t)
			return it.current();
	
	return 0;
}

	ContentLine *
VCard::contentLine(const QCString & s)
{
	parse();
	QListIterator<ContentLine> it(contentLineList_);
	
	for (; it.current(); ++it)
		if (it.current()->entityType() == s)
			return it.current();
	
	return 0;
}

}
