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
#include <kstdaction.h>

#include <qcursor.h>
#include <qtooltip.h>
#include <QIcon>
//Added by qt3to4:
#include <Q3PtrList>
#include <kiconloader.h>

HVItem::HVItem( QWidget *parent, const char *name )
	: BoxContainerItem( 0, name ),
	_label( new Label( parent, "label" ) ),
	_popup( new KMenu( _label ) ),
	_actions( new KActionCollection( _popup ) )
{
	_popup->addTitle( QIcon( qApp->windowIcon().pixmap(IconSize(KIcon::Small),IconSize(KIcon::Small)) ), kapp->caption() );
	this->fillKMenu( _popup, _actions );
	_popup->insertSeparator();
	KStdAction::quit( kapp, SLOT( quit() ), _actions )->plug( _popup );
	
	connect( _label, SIGNAL( mouseButtonPressed( Qt::ButtonState ) ), this, SLOT( mouseButtonPressed( Qt::ButtonState ) ) );
}

HVItem::~HVItem()
{
	//Let everything be deleted by his parents.
}
	
void HVItem::showBox()
{
	_label->show();
}
	
void HVItem::setCount( const int count, const bool newMessages )
{
	drawLabel( _label, count, newMessages );
}

void HVItem::setTooltip( const QString& string )
{
	QToolTip::add( _label, string );
}

void HVItem::slotShowPassivePopup( Q3PtrList< KornMailSubject >* list, int total, bool date, const QString& name )
{
	showPassivePopup( _label, list, total, name, date );
}

void HVItem::slotShowPassivePopup( const QString& errorMessage, const QString& name )
{
	KPassivePopup::message( QString( "korn-%1-%2" ).arg( objId().constData() ).arg( name ), errorMessage, _label, "Passive error message" );
}
	
void HVItem::doPopup()
{
	_popup->popup( QCursor::pos() );
}

#include "hvitem.moc"
