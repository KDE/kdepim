/*  -*- mode: C++; c-file-style: "gnu" -*-
    customactions.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "customactions.h"

#include <ktoolbar.h>
#include <kapplication.h>

#include <qlineedit.h>
#include <qlabel.h>


LabelAction::LabelAction( const QString & text,  KActionCollection * parent,
			  const char* name )
  : KAction( text, QIconSet(), KShortcut(), 0, 0, parent, name )
{

}

int LabelAction::plug( QWidget * widget, int index ) {
  if ( kapp && !kapp->authorizeKAction( name() ) )
    return -1;
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar * bar = (KToolBar *)widget;      
    int id_ = getToolButtonID();      
    QLabel* label = new QLabel( text(), bar, "kde toolbar widget" );
    bar->insertWidget( id_, label->width(), label, index );      
    addContainer( bar, id_ );      
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
    return containerCount() - 1;
  }
    
  return KAction::plug( widget, index );
}

LineEditAction::LineEditAction( const QString & text, KActionCollection * parent,
				QObject * receiver, const char * member, const char * name )
  : KAction( text, QIconSet(), KShortcut(), 0, 0, parent, name ), 
    _le(0), _receiver(receiver), _member(member)
{

}

int LineEditAction::plug( QWidget * widget, int index ) {
  if ( kapp && !kapp->authorizeKAction( name() ) )
    return -1;
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar *bar = (KToolBar *)widget;      
    int id_ = getToolButtonID();      
    // The toolbar trick doesn't seem to work for lineedits
    //_le = new QLineEdit( bar, "kde toolbar widget" );
    _le = new QLineEdit( bar );
    bar->insertWidget( id_, _le->width(), _le, index );      
    bar->setStretchableWidget( _le );
    addContainer( bar, id_ );      
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
    connect( _le, SIGNAL( returnPressed() ), _receiver, _member );      
    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}

void LineEditAction::clear() {
  _le->clear();
}

void LineEditAction::focusAll() {
  _le->selectAll();
  _le->setFocus();
}

QString LineEditAction::text() const {
  return _le->text();
}

void LineEditAction::setText( const QString & txt ) {
  _le->setText(txt);
}


ComboAction::ComboAction( const QStringList & lst,  KActionCollection * parent,
			  QObject * receiver, const char * member, const char * name )
  : KAction( QString::null, QIconSet(), KShortcut(), 0, 0, parent, name ), 
    _lst(lst), _receiver(receiver), _member(member)
{

}

int ComboAction::plug( QWidget * widget, int index ) {
  if ( kapp && !kapp->authorizeKAction( name() ) )
    return -1;
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar *bar = (KToolBar *)widget;      
    int id_ = getToolButtonID();   
    bar->insertCombo( _lst, id_, false, SIGNAL( highlighted(int) ), _receiver, _member ); 
    addContainer( bar, id_ );      
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );      
    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}

#include "customactions.moc"
