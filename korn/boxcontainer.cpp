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

#include "boxcontainer.h"
#include "boxcontaineritem.h"

#include <kconfig.h>
#include <kdebug.h>

#include <qptrlist.h>

BoxContainer::BoxContainer( QObject * parent, const char * name )
	: QObject( parent, name ),
	_items( new QPtrList< BoxContainerItem > )
{
	_items->setAutoDelete( true );
}

BoxContainer::~BoxContainer()
{
	delete _items;
}

void BoxContainer::readConfig( KConfig* config )
{
	int counter = 0;
	while( config->hasGroup( QString( "korn-%1" ).arg( counter ) ) )
	{
		config->setGroup( QString( "korn-%1" ).arg( counter ) );
		BoxContainerItem *item = newBoxInstance();
		item->readConfig( config, counter );
		addItem( item );
		++counter;
	}
}

void BoxContainer::writeConfig( KConfig *config )
{
	int index = 0;
	
	BoxContainerItem *item;
	for ( item = _items->first(); item; item = _items->next() )
	{
		item->writeConfig( config, index );
		++index;
	}
	
}

void BoxContainer::showBox()
{
	BoxContainerItem *item;
	for( item = _items->first(); item; item = _items->next() )
		item->showBox();
}

void BoxContainer::slotShowConfiguration()
{
	emit showConfiguration();
}

void BoxContainer::addItem( BoxContainerItem* item )
{
	connect( item, SIGNAL( showConfiguration() ), this, SLOT( slotShowConfiguration() ) );
	_items->append( item );
}

#include "boxcontainer.moc"
