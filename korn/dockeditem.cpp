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

#include "dockeditem.h"

#include "systemtray.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpassivepopup.h>
#include <kpopupmenu.h>
#include <klocale.h>

#include <tqbitmap.h>
#include <tqcursor.h>
#include <tqtooltip.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqmovie.h>

DockedItem::DockedItem( TQWidget * parent, const char * name )
	: BoxContainerItem( parent, name ),
	_systemtray( new SystemTray( parent, "System tray" ) )	
{
	this->fillKPopupMenu( _systemtray->contextMenu(), _systemtray->actionCollection() );
	
	connect( _systemtray, TQT_SIGNAL( quitSelected() ), kapp, TQT_SLOT( quit() ) );
	connect( _systemtray, TQT_SIGNAL( mouseButtonPressed( Qt::ButtonState ) ),
		 this, TQT_SLOT( mouseButtonPressed( Qt::ButtonState ) ) );
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

void DockedItem::setTooltip( const TQString& tooltip )
{
	TQToolTip::add( _systemtray, tooltip );
}

void DockedItem::slotShowPassivePopup( TQPtrList< KornMailSubject >* list, int total, bool date, const TQString& name )
{
	showPassivePopup( _systemtray, list, total, name, date );
}

void DockedItem::slotShowPassivePopup( const TQString& message, const TQString& name )
{
	KPassivePopup::message( i18n( "Korn - %1/%2" ).arg( objId() ).arg( name ), message, _systemtray, "Passive error message" );
}

void DockedItem::doPopup()
{
	_systemtray->contextMenu()->popup( TQCursor::pos() );
}

#include "dockeditem.moc"
