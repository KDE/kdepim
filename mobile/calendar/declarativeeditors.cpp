/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "ui_eventortodomobile.h"

DCollectionCombo::DCollectionCombo( QDeclarativeItem *parent )
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

DIEGeneral::DIEGeneral( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<MobileIncidenceGeneral,
                          IncidenceView,
                          &IncidenceView::setGeneralEditor>( parent )
{ }
