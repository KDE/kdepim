/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

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

#include "editorgeneral.h"

#include "ui_editorgeneral.h"

#include <Akonadi/Collection>

#include <KABC/Addressee>
#include <KABC/PhoneNumber>

static void fillPhoneTypeCombo( QComboBox *combo )
{
  static KABC::PhoneNumber::TypeFlag usedTypes[] =
    { KABC::PhoneNumber::Pref, KABC::PhoneNumber::Home, KABC::PhoneNumber::Work, KABC::PhoneNumber::Cell };

  for ( uint i = 0; i < sizeof( usedTypes ) / sizeof( KABC::PhoneNumber::TypeFlag ); ++i ) {
    combo->addItem( KABC::PhoneNumber::typeLabel( usedTypes[ i ] ), QVariant( usedTypes[ i ] ) );
  }
}

class EditorGeneral::Private
{
  EditorGeneral *const q;
 
  public:
    explicit Private( EditorGeneral *parent ) : q( parent )
    {
      mUi.setupUi( parent );

      fillPhoneTypeCombo( mUi.phone1Type );
      fillPhoneTypeCombo( mUi.phone2Type );
      mUi.phone2Type->setCurrentIndex( 1 );

      mUi.collectionSelector->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    }

  public:
    Ui::EditorGeneral mUi;

    KABC::Addressee mContact;

  public: // slots
    void nameTextChanged( const QString &text )
    {
      mContact.setNameFromString( text );
    }
};

EditorGeneral::EditorGeneral( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
  connect( d->mUi.fullName, SIGNAL( textChanged( QString ) ), SLOT( nameTextChanged( QString ) ) );

  connect( d->mUi.saveButton, SIGNAL( clicked() ), SIGNAL( saveClicked() ) );
  connect( d->mUi.collectionSelector, SIGNAL( currentChanged( Akonadi::Collection ) ),
           SIGNAL( collectionChanged( Akonadi::Collection ) ) );
}

EditorGeneral::~EditorGeneral()
{
  delete d;
}

void EditorGeneral::loadContact( const KABC::Addressee &contact )
{
  d->mContact = contact;

  d->mUi.fullName->blockSignals( true );
  d->mUi.fullName->setText( contact.assembledName() );
  d->mUi.fullName->blockSignals( false );
}

void EditorGeneral::saveContact( KABC::Addressee &contact )
{
  contact.setPrefix( d->mContact.prefix() );
  contact.setGivenName( d->mContact.givenName() );
  contact.setAdditionalName( d->mContact.additionalName() );
  contact.setFamilyName( d->mContact.familyName() );
  contact.setSuffix( d->mContact.suffix() );
}

void EditorGeneral::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mUi.collectionSelector->setDefaultCollection( collection );
}

#include "editorgeneral.moc"
