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

#ifndef MK_HVCONTAINER_H
#define MK_HVCONTAINER_H

#include "boxcontainer.h"

class BoxContainerItem;

class QHBox;

/**
 * This is a BoxContainer for both Horizontal and Vertical displays.
 */

class HVContainer : public BoxContainer
{ Q_OBJECT
public:
	/**
	 * Constructor: all elements are passed to BoxContainer, except orientation.
	 *
	 * @param orientation The orientation of the box: it is a vertical or horizontal box?
	 */
	HVContainer( Qt::Orientation orientation, QObject * parent = 0 , const char * name = 0 );
	~HVContainer();
	
	virtual void showBox();
protected:
	virtual BoxContainerItem* newBoxInstance() const;
private:
	QHBox *box;
};

#endif //MK_HVCONTAINER_H
