/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_DOCKEDCONTAINER_H
#define MK_DOCKEDCONTAINER_H

#include "boxcontainer.h"

class BoxContainerItem;

/**
 * This class is a implementation of a BoxContainer and does almost nothing.
 * The only thing it does is creating DocketItems.
 */
class DockedContainer : public BoxContainer
{ Q_OBJECT
public:
	DockedContainer( QObject * parent = 0, const char * name = 0 );
	~DockedContainer();
	
protected:
	//virtual void addItem( BoxContainerItem* ); //Overiding not neccesairy

	/**
	 * @return A new instance to a DocketItem.
	 */
	virtual BoxContainerItem* newBoxInstance() const;
};

#endif //MK_DOCKEDCONTAINER_H
