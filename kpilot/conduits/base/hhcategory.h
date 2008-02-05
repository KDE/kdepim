#ifndef HHCATEGORY_H
#define HHCATEGORY_H
/* hhcategory.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
*/
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <QtCore/QString>

#include "category.h"

/**
 * Class that contains additional category information stored on a handheld.
 */
class KPILOT_EXPORT HHCategory : public Category {

public:
	/**
	 * Creates the "Unfiled" category.
	 */
	HHCategory();

	HHCategory( QString name, bool renamed, int index, char id );
	
	HHCategory( const HHCategory &copy );

	int index() const;
	
	void setIndex( const int index );

	char id() const;
	
	void setId( const char id );
	
	bool renamed() const;
	
	void setRenamed( bool renamed );

private:
	bool fRenamed;
	int fIndex;
	char fId;
};

#endif
