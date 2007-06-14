/*
    customactions.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2001,2002,2004 Klar�vdalens Datakonsult AB

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include <kcombobox.h>
#include <ktoolbar.h>
#include <kauthorized.h>

#include <KLineEdit>
#include <QLabel>


LabelAction::LabelAction( const QString & text,  KActionCollection * parent,
			  const QString &name )
  : KAction( text, parent )
{
  parent->addAction( name, this );
}

QWidget* LabelAction::createWidget( QWidget * parent )
{
  QLabel *label = new QLabel( text(), parent );
  label->setObjectName( "kde toolbar widget" );
  return label;
}

#ifdef __GNUC__
#warning How to port the Kiosk stuff below?
#endif
/*int LabelAction::plug( QWidget * widget, int index ) {
  if ( !KAuthorized::authorizeKAction( name() ) )
    return -1;
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar * bar = (KToolBar *)widget;
    int id_ = getToolButtonID();
    QLabel* label = new QLabel( text(), bar );
    label->setObjectName( "kde toolbar widget" );
    bar->insertWidget( id_, label->width(), label, index );
    addContainer( bar, id_ );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}*/

LineEditAction::LineEditAction( const QString & text, KActionCollection * parent,
				QObject * receiver, const char * member, const QString & name )
  : KAction( text, parent ),
    _le(0), _receiver(receiver), _member(member)
{
  parent->addAction( name, this );
}

QWidget* LineEditAction::createWidget( QWidget * parent )
{
  _le = new KLineEdit( parent );
  _le->setClearButtonShown(true);
  connect( _le, SIGNAL( returnPressed() ), _receiver, _member );
  return _le;
}

void LineEditAction::deleteWidget(QWidget* widget)
{
  if ( widget == _le )
    _le = 0;
  delete widget;
}

#ifdef __GNUC__
#warning How to port the Kiosk stuff below?
#endif
/*int LineEditAction::plug( QWidget * widget, int index ) {
  if ( !KAuthorized::authorizeKAction( name() ) )
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
}*/

void LineEditAction::clear() {
  if ( _le )
    _le->clear();
}

void LineEditAction::focusAll() {
  if ( _le ) {
    _le->selectAll();
    _le->setFocus();
  }
}

QString LineEditAction::text() const {
  if ( _le )
    return _le->text();
  return QString();
}

void LineEditAction::setText( const QString & txt ) {
  if ( _le )
    _le->setText(txt);
}


ComboAction::ComboAction( const QStringList & lst,  KActionCollection * parent,
                          QObject * receiver, const char * member, const QString & name,
                          int selectedID )
  : KAction( QString(), parent ),
  _lst(lst), _receiver(receiver), _member(member), _selectedId( selectedID )
{
  parent->addAction( name, this );
}

QWidget* ComboAction::createWidget( QWidget * parent )
{
  KComboBox* box = new KComboBox( parent );
  box->addItems( _lst );
  connect( box, SIGNAL( highlighted(int) ), _receiver, _member );
  return box;
}

#ifdef __GNUC__
#warning How to port the Kiosk stuff below?
#endif
/*int ComboAction::plug( QWidget * widget, int index ) {
  if ( !KAuthorized::authorizeKAction( name() ) )
    return -1;
  if ( widget->inherits( "KToolBar" ) ) {
    KToolBar *bar = (KToolBar *)widget;
    int id_ = getToolButtonID();
    bar->insertCombo( _lst, id_, false, SIGNAL( highlighted(int) ), _receiver, _member );
    bar->setCurrentComboItem( id_,_selectedId );
    addContainer( bar, id_ );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    return containerCount() - 1;
  }

  return KAction::plug( widget, index );
}*/

#include "customactions.moc"
