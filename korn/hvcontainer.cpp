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

#include "hvcontainer.h"

#include "hvitem.h"

#include <kdebug.h>

#include <qvbox.h>

HVContainer::HVContainer( Qt::Orientation orientation, QObject * parent, const char * name )
	: BoxContainer( parent, name ),
	box( 0 )	
{
	if( orientation == Qt::Horizontal )
		box = new QHBox( 0, "hbox" );
	else
		box = new QVBox( 0, "vbox" );
}

HVContainer::~HVContainer()
{
	delete box;
}

void HVContainer::showBox()
{
	box->show();
}
	
BoxContainerItem* HVContainer::newBoxInstance() const
{
	return new HVItem( box, "horizontal/vertical item" );
}

#include "hvcontainer.moc"
