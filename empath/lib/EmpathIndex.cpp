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

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"
#include "EmpathMessageList.h"

EmpathIndex::EmpathIndex()
	:	QDict<EmpathIndexRecord>(101)
{
	// empty
}

EmpathIndex::~EmpathIndex()
{
	// empty
}

	int
EmpathIndex::compareItems(
	EmpathIndexRecord * item1,
	EmpathIndexRecord * item2)
{
	// Compare parent id of the two messages.
	Q_UINT32 i1 = ((EmpathIndexRecord *)item1)->date().asUnixTime();
	Q_UINT32 i2 = ((EmpathIndexRecord *)item2)->date().asUnixTime();

	// Follow true definition -
	// 0   if item1 == item2
	// > 0 if item1  > item2
	// < 0 if item1  < item2
	
	return i2 - i1;
}

	const EmpathIndexRecord *
EmpathIndex::messageDescription(const RMessageID & id) const
{
	EmpathIndexIterator it(*this);
	
	for (; it.current(); ++it)
		if (it.current()->messageID() == id) return it.current();

	return 0;
}

	bool
EmpathIndex::remove(const RMessageID & id)
{
	const EmpathIndexRecord * m = messageDescription(id);
	if (m == 0 || m->messageID() != id) return false;
	QDict::remove(m->id());
	return true;
}

	Q_UINT32
EmpathIndex::countUnread() const
{
	Q_UINT32 unread = 0;
	EmpathIndexIterator it(*this);
	for (; it.current(); ++it)
		if (it.current()->status() ^ Read) ++unread;
	return unread;
}

	void
EmpathIndex::sync()
{
	folder_->update();
}

	void
EmpathIndex::parseNewMail(EmpathMessageList * tempMessageList)
{
}

	QString
EmpathIndex::asString() const
{
	QString tempString;
	EmpathIndexIterator it(*this);
	for (; it.current(); ++it)
		tempString += it.current()->id();
	return tempString;
}

