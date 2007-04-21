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

#include "hvitem.h"

#include "label.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kpassivepopup.h>
#include <kmenu.h>
#include <kstandardaction.h>

#include <QCursor>
#include <QLabel>
#include <QList>

#include <QIcon>
//Added by qt3to4:
#include <kiconloader.h>

HVItem::HVItem( QWidget *parent )
	: BoxContainerItem( 0 ),
	_label( new Label( parent ) ),
	_popup( new KMenu( _label ) ),
	_actions( new KActionCollection( _popup ) )
{
	_popup->addTitle( QIcon( qApp->windowIcon().pixmap(IconSize(K3Icon::Small),IconSize(K3Icon::Small)) ), KGlobal::caption() );
	this->fillKMenu( _popup, _actions );
	_popup->addSeparator();
	_popup->addAction( KStandardAction::quit( kapp, SLOT( quit() ), _actions ) );
	
	connect( _label, SIGNAL( mouseButtonPressed( Qt::MouseButton ) ), this, SLOT( mouseButtonPressed( Qt::MouseButton ) ) );
}

HVItem::~HVItem()
{
	//Let everything be deleted by his parents.
}
	
void HVItem::showBox()
{
	_label->show();
}

QLabel* HVItem::getLabel() const
{
	return _label;
}
	
void HVItem::setCount( const int count, const bool newMessages )
{
	drawLabel( _label, count, newMessages );
}

void HVItem::setTooltip( const QString& string )
{
	_label->setToolTip( string );
}

void HVItem::slotShowPassivePopup( QList< KornMailSubject >* list, int total, bool date, const QString& name )
{
	showPassivePopup( _label, list, total, name, date );
}

void HVItem::slotShowPassivePopup( const QString& errorMessage, const QString& name )
{
#ifdef __GNUC__
#warning Port objId() usage!
#endif
	KPassivePopup::message( QString( "korn-%1-%2" ).arg( /*objId().constData()*/"" ).arg( name ), errorMessage, _label );
}
	
void HVItem::doPopup()
{
	_popup->popup( QCursor::pos() );
}

#include "hvitem.moc"
