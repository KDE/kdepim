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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "boxcontainer.h"
#include "boxcontaineritem.h"

#include <kconfig.h>
#include <kdebug.h>

#include <qlist.h>

BoxContainer::BoxContainer( QObject * parent )
	: QObject( parent ),
	_items( new QList< BoxContainerItem* > )
{
}

BoxContainer::~BoxContainer()
{
	while( !_items->isEmpty() )
		delete _items->takeFirst();
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
	for( int index = 0; index < _items->size(); ++index )
		_items->at( index )->writeConfig( config, index );
	
}

void BoxContainer::showBox()
{
	for( int xx = 0; xx < _items->size(); ++xx )
		_items->at( xx )->showBox();
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
