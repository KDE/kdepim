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
    combo->addItem( KABC::PhoneNumber::typeLabel( usedTypes[ i ] ), QVariant::fromValue<int>( usedTypes[ i ] ) );
  }
}

class PhoneWidgets
{
  public:
    PhoneWidgets( QLineEdit *input, QComboBox *type )
      : mInput( input ), mType( type )
    {
      fillPhoneTypeCombo( mType );
    }


  public:
    QString mId;
    QLineEdit *mInput;
    QComboBox *mType;
};

class EditorGeneral::Private
{
  EditorGeneral *const q;
 
  public:
    explicit Private( EditorGeneral *parent )
      : q( parent ),
        mDefaultPhoto( KIconLoader::global()->loadIcon( "user-identity", KIconLoader::Dialog, KIconLoader::SizeHuge ) )
    {
      mUi.setupUi( parent );

      mEmailInputs << mUi.email1;
      mEmailInputs << mUi.email2;
      mLastEmailRow = 2; // third row
      
      mPhoneWidgets << new PhoneWidgets( mUi.phone1, mUi.phone1Type );
      mPhoneWidgets << new PhoneWidgets( mUi.phone2, mUi.phone2Type );
      mUi.phone2Type->setCurrentIndex( 1 );
      mLastPhoneRow = 4; // fifth row

      mUi.collectionSelector->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );

      mUi.pictureButton->setIconSize( mDefaultPhoto.size() );
      mUi.pictureButton->setIcon( mDefaultPhoto );      
    }

    ~Private()
    {
      qDeleteAll( mPhoneWidgets );
    }

    void ensureEmailRows( int emailCount )
    {
      // TODO delete unnecessary rows?
      addEmailRows( emailCount );
    }

    void ensurePhoneRows( int phoneCount )
    {
      // TODO delete unnecessary rows?
      addPhoneRows( phoneCount );
    }

  public:
    Ui::EditorGeneral mUi;

    KABC::Addressee mContact;

    QList<QLineEdit*> mEmailInputs;
    QList<PhoneWidgets*> mPhoneWidgets;

    int mLastEmailRow;
    int mLastPhoneRow;

    const QPixmap mDefaultPhoto;
    
  public: // slots
    void nameTextChanged( const QString &text )
    {
      mContact.setNameFromString( text );
      mUi.saveButton->setEnabled( !text.trimmed().isEmpty() );
    }

    void addEmailClicked();
    void addPhoneClicked();

  private:
    void addEmailRows( int newRowCount );
    void addPhoneRows( int newRowCount );
};

void EditorGeneral::Private::addEmailClicked()
{
  addEmailRows( mEmailInputs.count() + 1 );
}

void EditorGeneral::Private::addPhoneClicked()
{
  addPhoneRows( mPhoneWidgets.count() + 1 );
}

void EditorGeneral::Private::addEmailRows( int newRowCount )
{
  if ( newRowCount <= mEmailInputs.count() ) {
    return;
  }

  // remove widgets from layout
  mUi.gridLayout->removeWidget( mUi.addEmailButton );

  QList<PhoneWidgets*>::const_iterator widgetIt = mPhoneWidgets.constBegin();
  for ( ; widgetIt != mPhoneWidgets.constEnd(); ++widgetIt ) {
    mUi.gridLayout->removeWidget( (*widgetIt)->mInput );
    mUi.gridLayout->removeWidget( (*widgetIt)->mType );
  }

  mUi.gridLayout->removeWidget( mUi.phoneLabel );
  mUi.gridLayout->removeWidget( mUi.addPhoneButton );
  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );

  int row = mLastEmailRow + 1;
  
  // add new widgets
  for ( ; mEmailInputs.count() < newRowCount; ++row, ++mLastEmailRow, ++mLastPhoneRow ) {
    QLineEdit *lineEdit = new QLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    mEmailInputs << lineEdit;
  }

  // re-add widgets
  mUi.gridLayout->addWidget( mUi.addEmailButton, mLastEmailRow, 2, 1, 1 );

  mUi.gridLayout->addWidget( mUi.phoneLabel, row, 0, 1, 1 );
  widgetIt = mPhoneWidgets.constBegin();
  for ( ; widgetIt != mPhoneWidgets.constEnd(); ++widgetIt, ++row ) {
    mUi.gridLayout->addWidget( (*widgetIt)->mInput, row, 1, 1, 1 );
    mUi.gridLayout->addWidget( (*widgetIt)->mType, row, 2, 1, 1 );
  }

  mUi.gridLayout->addWidget( mUi.addPhoneButton, mLastPhoneRow, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.saveButton, row, 1, 1, 2 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, row, 3, 1, 1 );
}

void EditorGeneral::Private::addPhoneRows( int newRowCount )
{
  if ( newRowCount <= mPhoneWidgets.count() ) {
    return;
  }

  // remove widgets from layout
  mUi.gridLayout->removeWidget( mUi.addPhoneButton );
  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );

  int row = mLastPhoneRow + 1;
  // add new widgets
  for ( ; mPhoneWidgets.count() < newRowCount; ++row, ++mLastPhoneRow ) {
    QLineEdit *lineEdit = new QLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    QComboBox *combo = new QComboBox( q );
    mUi.gridLayout->addWidget( combo, row, 2, 1, 1 );
    mPhoneWidgets << new PhoneWidgets( lineEdit, combo );
  }

  // re-add widgets
  mUi.gridLayout->addWidget( mUi.addPhoneButton, mLastPhoneRow, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.saveButton, mLastPhoneRow + 1, 1, 1, 2 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, mLastPhoneRow + 1, 3, 1, 1 );
}

EditorGeneral::EditorGeneral( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
  connect( d->mUi.fullName, SIGNAL( textChanged( QString ) ), SLOT( nameTextChanged( QString ) ) );

  connect( d->mUi.addEmailButton, SIGNAL( clicked() ), SLOT( addEmailClicked() ) );
  
  connect( d->mUi.addPhoneButton, SIGNAL( clicked() ), SLOT( addPhoneClicked() ) );
 
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

  const QStringList emails = contact.emails();
  d->ensureEmailRows( emails.count() );

  QList<QLineEdit*>::iterator inputIt = d->mEmailInputs.begin();
  Q_FOREACH( const QString &email, emails ) {
    (*inputIt)->setText( email );
    ++inputIt;
  }
  
  const KABC::PhoneNumber::List phones = contact.phoneNumbers();
  d->ensurePhoneRows( phones.count() );
  
  QList<PhoneWidgets*>::iterator widgetIt = d->mPhoneWidgets.begin();
  Q_FOREACH( const KABC::PhoneNumber &phone, phones ) {
    PhoneWidgets* widgets = *widgetIt;

    widgets->mId = phone.id();
    widgets->mInput->setText( phone.number() );

    // TODO type

    ++widgetIt;
  }

  QPixmap photo = d->mDefaultPhoto;
  if ( !contact.photo().isEmpty() ) {
    photo = QPixmap::fromImage( contact.photo().data() );
  }
  d->mUi.pictureButton->setIconSize( photo.size().boundedTo( d->mDefaultPhoto.size() ) );
  d->mUi.pictureButton->setIcon( photo );
}

void EditorGeneral::saveContact( KABC::Addressee &contact )
{
  contact.setPrefix( d->mContact.prefix() );
  contact.setGivenName( d->mContact.givenName() );
  contact.setAdditionalName( d->mContact.additionalName() );
  contact.setFamilyName( d->mContact.familyName() );
  contact.setSuffix( d->mContact.suffix() );

  Q_FOREACH( QLineEdit *input, d->mEmailInputs ) {
    const QString email = input->text().trimmed();
    if ( !email.isEmpty() ) {
      contact.insertEmail( email );
    }
  }
  
  Q_FOREACH( PhoneWidgets *widgets, d->mPhoneWidgets ) {
    const QString number = widgets->mInput->text().trimmed();
    const QVariant typeVar = widgets->mType->itemData( widgets->mType->currentIndex() );
    if ( !number.isEmpty() ) {
      KABC::PhoneNumber phone( number, (KABC::PhoneNumber::TypeFlag)typeVar.value<int>() );
      if ( !widgets->mId.isEmpty() ) {
        phone.setId( widgets->mId );
      }
      contact.insertPhoneNumber( phone );
    }
  }
}

Akonadi::Collection EditorGeneral::selectedCollection() const
{
  return d->mUi.collectionSelector->currentCollection();
}

void EditorGeneral::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mUi.collectionSelector->setDefaultCollection( collection );
}

#include "editorgeneral.moc"
