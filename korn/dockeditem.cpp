/*
 * Copyright (C) 2004-2006, Mart Kelder (mart@kelder31.nl)
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

#include "settings.h"
#include "systemtray.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpassivepopup.h>
#include <kmenu.h>

#include <QBitmap>
#include <QCursor>

#include <QPainter>
#include <QPixmap>
#include <QMovie>

DockedItem::DockedItem( QWidget * parent )
	: BoxContainerItem( parent ),
	_systemtray( new SystemTray( parent ) )
{
	_systemtray->setObjectName( "System tray" );
        KMenu *menu = new KMenu(parent);
	this->fillKMenu( menu, _systemtray->actionCollection() );
        _systemtray->setContextMenu( menu );

	connect( _systemtray, SIGNAL( quitSelected() ), kapp, SLOT( quit() ) );
	connect( _systemtray, SIGNAL( mouseButtonPressed( Qt::MouseButton ) ),
		 this, SLOT( mouseButtonPressed( Qt::MouseButton ) ) );
}

DockedItem::~DockedItem()
{
	delete _systemtray;
}

void DockedItem::showBox()
{
	_systemtray->show();
}

void DockedItem::readConfig( BoxSettings* config, BoxSettings *config_settings, const int index )
{
	BoxContainerItem::readConfig( config, config_settings, index );

	//No additional information to be loaded.
}

void DockedItem::setCount( const int count, const bool newMessages )
{
	QPixmap pixmap( 22, 22 );
	bool isEmpty = makePixmap( pixmap, count, newMessages );

	_systemtray->setVisible( !isEmpty );

	if( !isEmpty )
		_systemtray->setIcon( QIcon( pixmap ) );
}

void DockedItem::setTooltip( const QString& tooltip )
{
	_systemtray->setToolTip( tooltip );
}

void DockedItem::slotShowPassivePopup( QList< KornMailSubject >* list, int total, bool date, const QString& name )
{
        showPassivePopup( _systemtray->parentWidget(), list, total, name, date );
}

void DockedItem::slotShowPassivePopup( const QString& message, const QString& name )
{
	KPassivePopup::message( QString( "Korn - %1/%2" ).arg( _settings->boxName() ).arg( name ), message, _systemtray->parentWidget() );
}

void DockedItem::doPopup()
{
	_systemtray->contextMenu()->popup( QCursor::pos() );
}

#include "dockeditem.moc"
