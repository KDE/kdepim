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
#include "settings.h"

#include <kconfig.h>
#include <kdebug.h>

#include <QList>

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

void BoxContainer::readConfig( Settings *settings, Settings *config_settings )
{
	int counter = 0;
	while( settings->getBox( counter ) )
	{
		BoxContainerItem *item = newBoxInstance();
		item->readConfig( settings->getBox( counter ), config_settings->getBox( counter ), counter );
		addItem( item );
		++counter;
	}
}

void BoxContainer::writeConfig( Settings *settings )
{
	for( int index = 0; index < _items->size(); ++index )
		if( settings->getBox( index ) )
			_items->at( index )->writeConfig( settings->getBox( index ), index );
		else
			kWarning() <<"Could not update information: configuration file does not match real number of boxes";
	
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
