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

#ifndef EMPATHMESSAGELISTITEM_H
#define EMPATHMESSAGELISTITEM_H

// Qt includes
#include <qstring.h>
#include <qlistview.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"

class EmpathMessageListWidget;

/**
 * @internal
 */
class EmpathMessageListItem : public QListViewItem
{
	public:
	
		EmpathMessageListItem(
			EmpathMessageListWidget * parent,
			EmpathIndexRecord * msgDesc);

		EmpathMessageListItem(
			EmpathMessageListItem * parent,
			EmpathIndexRecord * msgDesc);

		~EmpathMessageListItem();
		
		virtual void setup();
#if QT_VERSION >= 200
		QString key(int, bool) const;
#else
		const char * key(int, bool) const;
#endif

		EmpathIndexRecord * msgDesc() const { return msgDesc_; }

		const RMessageID & messageID() const	{ return msgDesc_->messageID(); }
		
		const RMessageID & parentID() const		{ return msgDesc_->parentID(); }

		QString subject() const					{ return msgDesc_->subject(); }
		
		const RMailbox & sender() const			{ return msgDesc_->sender(); }
		
		const RDateTime & date() const			{ return msgDesc_->date(); }
		
		MessageStatus status() const			{ return msgDesc_->status(); }

		QString size() const;

		const char * className()				{ return "EmpathMessageListItem"; }
		
	private:

		EmpathIndexRecord * msgDesc_;

};

#endif

