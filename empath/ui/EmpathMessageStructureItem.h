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
# pragma interface "EmpathMessageStructureItem.h"
#endif

#ifndef EMPATHMESSAGESTRUCTUREITEM_H
#define EMPATHMESSAGESTRUCTUREITEM_H

// Qt includes
#include <qstring.h>
#include <qlistview.h>

// Local includes
#include "EmpathDefines.h"
#include <RMM_BodyPart.h>
#include <RMM_Enum.h>

/**
 * @internal
 */
class EmpathMessageStructureItem : public QListViewItem
{
	public:
	
		EmpathMessageStructureItem(QListView * parent, RMM::RBodyPart &);

		EmpathMessageStructureItem(
			EmpathMessageStructureItem * parent,
			RMM::RBodyPart &);

		~EmpathMessageStructureItem();
		
		virtual void setup();
		
		RMM::RBodyPart * part() { return &part_; }

		const char * className() const { return "EmpathStructureListItem"; }
		
	private:

		void _init();
		
		RMM::RBodyPart part_;
};

#endif

