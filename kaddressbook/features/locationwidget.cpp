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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbox.h>

#include <kabc/addressbook.h>
#include <kaccelmanager.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "core.h"
#include "locationconfig.h"

#include "locationwidget.h"

class LocationFactory : public KAB::ExtensionFactory
{
  public:
    KAB::ExtensionWidget *extension( KAB::Core *core, QWidget *parent, const char *name )
    {
      return new LocationWidget( core, parent, name );
    }

    KAB::ConfigureWidget *configureWidget( QWidget *parent, const char *name )
    {
      return new LocationConfigWidget( 0, parent, name );
    }

    bool configureWidgetAvailable() { return true; }

    QString identifier() const
    {
      return "contact_location";
    }
};

extern "C" {
  void *init_libkaddrbk_location()
  {
    return ( new LocationFactory );
  }
}

LocationWidget::LocationWidget( KAB::Core *core, QWidget *parent, const char *name )
  : KAB::ExtensionWidget( core, parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 4, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  mURLTypeCombo = new KComboBox( this );
  topLayout->addWidget( mURLTypeCombo, 0, 0 );

  QLabel *label = new QLabel( i18n( "Address type:" ), this );
  topLayout->addWidget( label, 0, 1 );

  mAddressTypeCombo = new KComboBox( this );
  label->setBuddy( mAddressTypeCombo );
  topLayout->addWidget( mAddressTypeCombo, 0, 2 );

  mLoadButton = new QPushButton( i18n( "Load..." ), this );
  mLoadButton->setEnabled( false );
  connect( mLoadButton, SIGNAL( clicked() ), SLOT( loadLocationPage() ) );
  topLayout->addWidget( mLoadButton, 0, 3 );

  QVBox *panel = new QVBox( this );
  topLayout->addMultiCellWidget( panel, 1, 1, 0, 3 );

  KAcceleratorManager::manage( this );

  KConfig config( "kaddressbookrc" );
  config.setGroup( QString( "Extensions_%1" ).arg( identifier() ) );

  mURLTypeCombo->insertStringList( config.readListEntry( "URLs" ) );
}

LocationWidget::~LocationWidget()
{
}

void LocationWidget::contactsSelectionChanged()
{
  mAddressList.clear();

  int pos = mAddressTypeCombo->currentItem();
  mAddressTypeCombo->clear();

  if ( contactsSelected() ) {
    KABC::Addressee::List list = selectedContacts();
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
  if ( mURLTypeCombo->count() == 0 ) {
    KMessageBox::sorry( this, i18n( "You have to configure the location widget first!" ) );
    return;
  }

  KURL url = createUrl( mAddressList[ mAddressTypeCombo->currentItem() ] );
  kapp->invokeBrowser( url.url() );
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

  // we need a unique identifier for map24.de
  KConfig config( "kaddressbookrc" );
  config.setGroup( QString( "Extensions_%1" ).arg( identifier() ) );
  QString uid = config.readEntry( "UID", 
                                  QDateTime::currentDateTime().toString() );

  QString urlTemplate = config.readEntry( mURLTypeCombo->currentText() );

#if KDE_VERSION >= 319
  return urlTemplate.replace( "%s", addr.street() ).
                     replace( "%r", addr.region() ).
                     replace( "%l", addr.locality() ).
                     replace( "%z", addr.postalCode() ).
                     replace( "%c", addr.countryToISO( addr.country() ) ).
                     replace( "%i", uid );
#else
  return urlTemplate.replace( "%s", addr.street() ).
                     replace( "%r", addr.region() ).
                     replace( "%l", addr.locality() ).
                     replace( "%z", addr.postalCode() ).
                     replace( "%c", "" ).
                     replace( "%i", uid );
#endif
}

#include "locationwidget.moc"
