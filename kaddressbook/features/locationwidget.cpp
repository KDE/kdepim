/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbox.h>

#include <kaccelmanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>

#include <kabc/addressbook.h>

#include "locationwidget.h"

class LocationFactory : public ExtensionFactory
{
  public:
    ExtensionWidget *create( ViewManager *vm, QWidget *parent )
    {
      return new LocationWidget( vm, parent );
    }
};

extern "C" {
  void *init_libkaddrbk_location()
  {
    return ( new LocationFactory );
  }
}

LocationWidget::LocationWidget( ViewManager *vm, QWidget *parent )
  : ExtensionWidget( vm, parent )
{
  QGridLayout *topLayout = new QGridLayout( this, 3, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Address Type:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mAddressTypeCombo = new KComboBox( this );
  label->setBuddy( mAddressTypeCombo );
  topLayout->addWidget( mAddressTypeCombo, 0, 1 );

  mLoadButton = new QPushButton( i18n( "Load..." ), this );
  mLoadButton->setEnabled( false );
  connect( mLoadButton, SIGNAL( clicked() ), SLOT( loadLocationPage() ) );
  topLayout->addWidget( mLoadButton, 0, 2 );

  QVBox *panel = new QVBox( this );
  topLayout->addMultiCellWidget( panel, 1, 1, 0, 2 );

  mHTMLPart = new KHTMLPart( panel );

  KAcceleratorManager::manage( this );
}

LocationWidget::~LocationWidget()
{
}

void LocationWidget::addresseeSelectionChanged()
{
  mAddressList.clear();

  int pos = mAddressTypeCombo->currentItem();
  mAddressTypeCombo->clear();

  if ( addresseesSelected() ) {
    KABC::Addressee::List list = selectedAddressees();
    mAddressList = list[0].addresses();
  }

  KABC::Address::List::Iterator it;
  for ( it = mAddressList.begin(); it != mAddressList.end(); ++it )
    mAddressTypeCombo->insertItem( (*it).typeLabel( (*it).type() ) );

  mAddressTypeCombo->setCurrentItem( pos );

  mLoadButton->setEnabled( mAddressList.count() > 0 );
}

QString LocationWidget::title() const
{
  return i18n( "Location of Contact" );
}

QString LocationWidget::identifier() const
{
  return "contact_location";
}

void LocationWidget::loadLocationPage()
{
  KURL url = createUrl( mAddressList[ mAddressTypeCombo->currentItem() ] );
  mHTMLPart->openURL( url );
}

QString LocationWidget::createUrl( const KABC::Address &addr )
{
  /**
    This method makes substitutions for the following place holders:
      %s street
      %r region
      %l locality
      %z zip code
      %c country (in ISO format)
   */

  QString map24Template = "http://map24.de/map24/index.php3?street0=%s&zip0=%z&city0=%l&country0=%c&force_maptype=RELOAD";

  return map24Template.replace( "%s", addr.street() ).
                       replace( "%r", addr.region() ).
                       replace( "%l", addr.locality() ).
                       replace( "%z", addr.postalCode() ).
                       replace( "%c", addr.countryToISO( addr.street() ) );
}

#include "locationwidget.moc"
