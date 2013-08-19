/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "declarativeeditors.h"

#include "ui_dialogmobile.h"
#include "ui_dialogmoremobile.h"

#include <QtCore/QTimer>

DCollectionCombo::DCollectionCombo( QGraphicsItem *parent )
  : DeclarativeWidgetBase<Akonadi::CollectionComboBox,
                          IncidenceView,
                          &IncidenceView::setCollectionCombo>( parent )
{ }

MobileIncidenceGeneral::MobileIncidenceGeneral( QWidget *parent )
  : QWidget( parent ), mUi( new Ui::EventOrTodoDesktop )
{
  mUi->setupUi( this );
}

MobileIncidenceGeneral::~MobileIncidenceGeneral()
{
  delete mUi;
}

DIEGeneral::DIEGeneral( QGraphicsItem *parent )
  : DeclarativeWidgetBase<MobileIncidenceGeneral,
                          IncidenceView,
                          &IncidenceView::setGeneralEditor>( parent )
{
  QTimer::singleShot( 0, this, SLOT(hack()) );
}

void DIEGeneral::hack()
{
  // For whatever reason, this is needed to make the widget resize correctly. If
  // someone finds a nices way to make this actually work....
  //
  // Without this hack the widget gets a width that is too large for the app. It
  // doesn't seem to matter which size you pass here.
  widget()->resize( QSize() );
}

MobileIncidenceMore::MobileIncidenceMore( QWidget *parent )
  : QStackedWidget( parent ), mUi( new Ui::EventOrTodoMore )
{
  mUi->setupUi( this );
}

MobileIncidenceMore::~MobileIncidenceMore()
{
  delete mUi;
}

DIEMore::DIEMore( QGraphicsItem *parent )
  : DeclarativeWidgetBase<MobileIncidenceMore,
                          IncidenceView,
                          &IncidenceView::setMoreEditor>( parent )
{ }

int DIEMore::currentIndex() const
{
  return widget()->currentIndex();
}

void DIEMore::setCurrentIndex( int index )
{
  widget()->setCurrentIndex( index );
}

#include "declarativeeditors.moc"
