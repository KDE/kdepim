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

#include "dockeditem.h"

#include "systemtray.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

#include <qbitmap.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qmovie.h>

DockedItem::DockedItem( QWidget * parent, const char * name )
	: BoxContainerItem( parent, name ),
	_systemtray( new SystemTray( parent, "System tray" ) )	
{
	this->fillKPopupMenu( _systemtray->contextMenu(), _systemtray->actionCollection() );
	
	connect( _systemtray, SIGNAL( quitSelected() ), kapp, SLOT( quit() ) );
	connect( _systemtray, SIGNAL( mouseButtonPressed( Qt::ButtonState ) ),
		 this, SLOT( mouseButtonPressed( Qt::ButtonState ) ) );
}

DockedItem::~DockedItem()
{
	delete _systemtray;
}

void DockedItem::showBox()
{
	_systemtray->show();
}
	
void DockedItem::readConfig( KConfig* config, const int index )
{
	BoxContainerItem::readConfig( config, index );
	
	//No additional information to be loaded.
}
	
void DockedItem::setCount( const int count, const bool newMessages )
{
	drawLabel( _systemtray, count, newMessages );
}

void DockedItem::setTooltip( const QString& tooltip )
{
	QToolTip::add( _systemtray, tooltip );
}

void DockedItem::slotShowPassivePopup( QPtrList< KornMailSubject >* list, int total, bool date, const QString& name )
{
	showPassivePopup( _systemtray, list, total, name, date );
}

void DockedItem::doPopup()
{
	_systemtray->contextMenu()->popup( QCursor::pos() );
}

#include "dockeditem.moc"
