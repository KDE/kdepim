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

#include "editormore.h"

#include "ui_editormore.h"
#include "ui_editormore_namepage.h"
#include "ui_editormore_internetpage.h"

#include <KABC/Addressee>

#include <QtCore/QSignalMapper>
#include <QtGui/QLabel>

class EditorMore::Private
{
  EditorMore *const q;

  public:
    explicit Private( EditorMore *parent ) : q( parent )
    {
      mUi.setupUi( parent );

      mMapper = new QSignalMapper( q );
      mMapper->setMapping( mUi.namePageButton, 0 );
      mMapper->setMapping( mUi.internetPageButton, 1 );
      mMapper->setMapping( mUi.personalPageButton, 2 );
      mMapper->setMapping( mUi.customFieldsPageButton, 3 );

      QWidget *namePage = new QWidget;
      mNamePage.setupUi( namePage );

      QWidget *internetPage = new QWidget;
      mInternetPage.setupUi( internetPage );

      QWidget *personalPage = new QLabel( "Personal Page" );
      QWidget *customFieldsPage = new QLabel( "CustomFields Page" );

      mUi.pageWidget->insertWidget( 0, namePage );
      mUi.pageWidget->insertWidget( 1, internetPage );
      mUi.pageWidget->insertWidget( 2, personalPage );
      mUi.pageWidget->insertWidget( 3, customFieldsPage );

      connect( mUi.namePageButton, SIGNAL( clicked() ),
               mMapper, SLOT( map() ) );
      connect( mUi.internetPageButton, SIGNAL( clicked() ),
               mMapper, SLOT( map() ) );
      connect( mUi.personalPageButton, SIGNAL( clicked() ),
               mMapper, SLOT( map() ) );
      connect( mUi.customFieldsPageButton, SIGNAL( clicked() ),
               mMapper, SLOT( map() ) );
      connect( mMapper, SIGNAL( mapped( int ) ),
               mUi.pageWidget, SLOT( setCurrentIndex( int ) ) );

      mUi.pageWidget->setCurrentIndex( 0 );

      connect( mNamePage.namePartsWidget, SIGNAL( nameChanged( const KABC::Addressee& ) ),
               mNamePage.displayNameWidget, SLOT( changeName( const KABC::Addressee& ) ) );
      connect( mNamePage.namePartsWidget, SIGNAL( nameChanged( const KABC::Addressee& ) ),
               q, SIGNAL( nameChanged( const KABC::Addressee& ) ) );
    }

  public:
    Ui::EditorMore mUi;
    Ui::NamePage mNamePage;
    Ui::InternetPage mInternetPage;
    QSignalMapper *mMapper;

    KABC::Addressee mContact;
};

static QString loadCustom( const KABC::Addressee &contact, const QString &key )
{
  return contact.custom( QLatin1String( "KADDRESSBOOK" ), key );
}

static void storeCustom( KABC::Addressee &contact, const QString &key, const QString &value )
{
  if ( value.isEmpty() )
    contact.removeCustom( QLatin1String( "KADDRESSBOOK" ), key );
  else
    contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), key, value );
}


EditorMore::EditorMore( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
}

EditorMore::~EditorMore()
{
  delete d;
}

void EditorMore::loadContact( const KABC::Addressee &contact )
{
  // name page
  d->mNamePage.nicknameLineEdit->setText( contact.nickName() );
  d->mNamePage.namePartsWidget->loadContact( contact );
  d->mNamePage.displayNameWidget->loadContact( contact );

  // internet page
  d->mInternetPage.urlLineEdit->setText( contact.url().url() );
  d->mInternetPage.blogLineEdit->setText( loadCustom( contact, QLatin1String( "BlogFeed" ) ) );
  d->mInternetPage.messagingLineEdit->setText( loadCustom( contact, QLatin1String( "X-IMAddress" ) ) );
}

void EditorMore::saveContact( KABC::Addressee &contact ) const
{
  // name page
  contact.setNickName( d->mNamePage.nicknameLineEdit->text() );
  d->mNamePage.namePartsWidget->storeContact( contact );
  d->mNamePage.displayNameWidget->storeContact( contact );

  // internet page
  contact.setUrl( d->mInternetPage.urlLineEdit->text() );
  storeCustom( contact, QLatin1String( "BlogFeed" ), d->mInternetPage.blogLineEdit->text() );
  storeCustom( contact, QLatin1String( "X-IMAddress" ), d->mInternetPage.messagingLineEdit->text() );
}

void EditorMore::updateOrganization( const QString &organization )
{
  d->mNamePage.displayNameWidget->changeOrganization( organization );
}

void EditorMore::updateName( const KABC::Addressee &contact )
{
  // this slot is called when the name has been changed in the 'General' page
  blockSignals( true );
  d->mNamePage.namePartsWidget->loadContact( contact );
  d->mNamePage.displayNameWidget->changeName( contact );
  blockSignals( false );
}

#include "editormore.moc"
