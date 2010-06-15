/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "editorlocation.h"

#include "locationmodel.h"
#include "ui_editorlocation.h"

#include <KABC/Addressee>

#include <QtGui/QDataWidgetMapper>

class EditorLocation::Private
{
  EditorLocation *const q;

  public:
    explicit Private( EditorLocation *parent ) : q( parent )
    {
      mUi.setupUi( parent );
      mModel = new LocationModel( q );

      mUi.addressSelectionCombo->setModel( mModel );
      mUi.addressSelectionCombo->setModelColumn( 0 );

      mMapper = new QDataWidgetMapper( q );
      mMapper->setModel( mModel );
      mMapper->addMapping( mUi.streetLineEdit, 1 );
      mMapper->addMapping( mUi.postOfficeBoxLineEdit, 2 );
      mMapper->addMapping( mUi.localityLineEdit, 3 );
      mMapper->addMapping( mUi.regionLineEdit, 4 );
      mMapper->addMapping( mUi.postalCodeLineEdit, 5 );
      mMapper->addMapping( mUi.countryLineEdit, 6 );
      mMapper->addMapping( mUi.editLabelLineEdit, 7 );
      mMapper->toFirst();

      q->connect( mUi.addressSelectionCombo, SIGNAL( activated( int ) ),
                  mMapper, SLOT( setCurrentIndex( int ) ) );
      q->connect( mUi.addAddressButton, SIGNAL( clicked() ),
                  SLOT( addAddress() ) );
      q->connect( mUi.deleteAddressButton, SIGNAL( clicked() ),
                  SLOT( removeAddress() ) );
    }

    void addAddress()
    {
      //TODO: show new dialog to ask for type
      mModel->insertRows( 0, 1 );
    }

    void removeAddress()
    {
      mModel->removeRows( mMapper->currentIndex(), 1 );
    }

  public:
    Ui::EditorLocation mUi;

    KABC::Addressee mContact;
    LocationModel *mModel;
    QDataWidgetMapper *mMapper;
};


EditorLocation::EditorLocation( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorLocation::~EditorLocation()
{
  delete d;
}

void EditorLocation::loadContact( const KABC::Addressee &contact )
{
}

void EditorLocation::saveContact( KABC::Addressee &contact )
{
}

#include "editorlocation.moc"
