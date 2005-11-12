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

#include "hvcontainer.h"

#include "hvitem.h"

#include <kdebug.h>

#include <qboxlayout.h>
#include <qwidget.h>

HVContainer::HVContainer( Qt::Orientation orientation, QObject * parent, const char * name )
	: BoxContainer( parent, name ),
	widget( 0 ),
	layout( 0 )	
{
	widget = new QWidget();
	if( orientation == Qt::Horizontal )
		layout = new QHBoxLayout();
	else
		layout = new QVBoxLayout();
	widget->setLayout( layout );
}

HVContainer::~HVContainer()
{
	delete layout;
	delete widget;
}

void HVContainer::showBox()
{
	widget->resize( widget->sizeHint() );
	widget->show();
}
	
BoxContainerItem* HVContainer::newBoxInstance() const
{
	HVItem *item = new HVItem( widget, "horizontal/vertical item" );
	layout->addWidget( item->getLabel(), 0 );
	return item;
}

#include "hvcontainer.moc"
