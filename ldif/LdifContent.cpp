/*
	libldif - LDAP LDIF parsing library

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

#include <qstrlist.h>
#include <RToken.h>
#include "ldif.h"

using namespace LDIF;


LdifContent::LdifContent()
	:	Entity()
{
	attrValRecList_.setAutoDelete(true);
}

LdifContent::LdifContent(const LdifContent & x)
	:	Entity(x)
{
	attrValRecList_.setAutoDelete(true);
}

LdifContent::LdifContent(const QCString & s)
	:	Entity(s)
{
	attrValRecList_.setAutoDelete(true);
}

	LdifContent &
LdifContent::operator = (LdifContent & x)
{
	if (*this == x) return *this;
	
	x.parse();
	
	attrValRecList_	= x.attrValRecList_;
	versionSpec_	= x.versionSpec_;

	Entity::operator = (x);
	return *this;
}

	LdifContent &
LdifContent::operator = (const QCString & s)
{
	Entity::operator = (s);
	return *this;
}

	bool
LdifContent::operator == (LdifContent & x)
{
	x.parse();
	return (versionSpec_ == x.versionSpec_ /* && ..... */); // TODO
}

LdifContent::~LdifContent()
{
}

	void
LdifContent::_parse()
{
	// We need to handle 1*(blank lines) as a record separator
	// We also need to ignore comments (^#)
	
	QCString versionStr;
	
	int start = 0;
	int lineEnd = strRep_.find('\n');
	
	if (lineEnd == -1)
		return; // Invalid - no lines !
	
	QCString buf;
	
	bool inRecord = false;
	
	while (lineEnd != -1) {
		
		QCString s = strRep_.mid(start, lineEnd - start);
		
		if (!s.isEmpty() && s.at(s.length() - 1) == '\r')
			s.truncate(s.length() - 1);

		if (s[0] == '#') {
			lDebug("Comment");
			start = lineEnd + 1;
			lineEnd = strRep_.find('\n', start);
			continue;
		}
		
		if (s.left(7) == "version") {
			lDebug("Found version");
			versionSpec_ = s;
			start = lineEnd + 1;
			lineEnd = strRep_.find('\n', start);
			continue;
		}
		
		if (s.isEmpty()) {
			
			if (inRecord) {
			
				inRecord = false;
				LdifAttrValRec * r = new LdifAttrValRec(buf);
				attrValRecList_.append(r);
				r->parse();
				buf.truncate(0);
				start = lineEnd + 1;
				lineEnd = strRep_.find('\n', start);
				continue;

			} else {
				
				start = lineEnd + 1;
				lineEnd = strRep_.find('\n', start);
				continue;
			}
		}
		
		inRecord = true;
		buf += s + '\n';
		
		start = lineEnd + 1;
		lineEnd = strRep_.find('\n', start);
	}
}

	void
LdifContent::_assemble()
{
	strRep_ = versionSpec_.asString() + '\n';
	
	LdifAttrValRecIterator it(attrValRecList_);
	
	for (; it.current(); ++it)
		strRep_ += it.current()->asString() + '\n';
}

