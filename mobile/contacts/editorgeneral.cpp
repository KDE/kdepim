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

#include <AkonadiCore/Collection>

#include <KContacts/Addressee>
#include <KContacts/PhoneNumber>

#include <KIconLoader>

class PhoneWidgets
{
  public:
    PhoneWidgets( MobileLineEdit *input, PhoneTypeCombo *type )
      : mInput( input ), mType( type )
    {
    }


  public:
    QString mId;
    MobileLineEdit *mInput;
    PhoneTypeCombo *mType;
};

class EditorGeneral::Private
{
  EditorGeneral *const q;

  public:
    explicit Private( EditorGeneral *parent )
      : q( parent )
    {
      mUi.setupUi( parent );

      QObject::connect( mUi.email1, SIGNAL(clearClicked()), q, SLOT(clearEmailClicked()) );
      QObject::connect( mUi.email2, SIGNAL(clearClicked()), q, SLOT(clearEmailClicked()) );
      mEmailInputs << mUi.email1;
      mEmailInputs << mUi.email2;
      mLastEmailRow = 2; // third row

      QObject::connect( mUi.phone1, SIGNAL(clearClicked()), q, SLOT(clearPhoneClicked()) );
      QObject::connect( mUi.phone2, SIGNAL(clearClicked()), q, SLOT(clearPhoneClicked()) );
      mPhoneWidgets << new PhoneWidgets( mUi.phone1, mUi.phone1Type );
      mUi.phone1Type->setType( KContacts::PhoneNumber::Pref );
      mPhoneWidgets << new PhoneWidgets( mUi.phone2, mUi.phone2Type );
      mLastPhoneRow = 4; // fifth row

      mUi.collectionSelector->setMimeTypeFilter( QStringList() << KContacts::Addressee::mimeType() );
      mUi.collectionSelector->setAccessRightsFilter( Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanChangeItem );

      mUi.gridLayout->removeWidget( mUi.pictureButton );
      mUi.gridLayout->addWidget( mUi.pictureButton, 0, 3, 3, 1, Qt::AlignTop );
      mUi.pictureButton->setType( ImageWidget::Photo );

      QObject::connect( mUi.launchAccountWizardButton, SIGNAL(clicked()), q, SIGNAL(requestLaunchAccountWizard()) );

      availableCollectionsChanged();
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

    KContacts::Addressee mContact;

    QList<MobileLineEdit*> mEmailInputs;
    QList<PhoneWidgets*> mPhoneWidgets;

    int mLastEmailRow;
    int mLastPhoneRow;

  public: // slots
    void nameTextChanged( const QString &text )
    {
      mContact.setNameFromString( text );
      mUi.saveButton->setEnabled( !text.trimmed().isEmpty() );

      emit q->nameChanged( mContact );
    }

    void addEmailClicked();
    void clearEmailClicked();
    void addPhoneClicked();
    void clearPhoneClicked();

    void availableCollectionsChanged()
    {
      const bool available = mUi.collectionSelector->currentCollection().isValid();
      mUi.collectionSelector->setVisible( available );
      mUi.launchAccountWizardButton->setVisible( !available );
    }

    void disableSaveButton()
    {
      mUi.saveButton->setEnabled( false );
    }

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
  mUi.gridLayout->removeWidget( mUi.cancelButton );
  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );
  mUi.gridLayout->removeWidget( mUi.launchAccountWizardButton );

  int row = mLastEmailRow + 1;

  // add new widgets
  for ( ; mEmailInputs.count() < newRowCount; ++row, ++mLastEmailRow, ++mLastPhoneRow ) {
    MobileLineEdit *lineEdit = new MobileLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    QObject::connect( lineEdit, SIGNAL(clearClicked()), q, SLOT(clearEmailClicked()) );
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
  mUi.gridLayout->addWidget( mUi.cancelButton, row, 2, 1, 1 );
  mUi.gridLayout->addWidget( mUi.saveButton, row, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, row, 1, 1, 1 );
  mUi.gridLayout->addWidget( mUi.launchAccountWizardButton, row, 1, 1, 1 );
}

void EditorGeneral::Private::clearEmailClicked()
{
  int index = 0;
  for ( ; index < mEmailInputs.count(); ++index ) {
    if ( mEmailInputs[ index ] == q->sender() ) {
      break;
    }
  }

  Q_ASSERT( index >= 0 && index < mEmailInputs.count() );

  // shift data
  for ( int i = index + 1; i < mEmailInputs.count(); ++i ) {
    mEmailInputs[ i - 1 ]->setText( mEmailInputs[ i ]->text() );
  }

  MobileLineEdit *last = mEmailInputs.last();
  if ( mEmailInputs.count() > 2 ) {
    // remove widgets from layout
    mUi.gridLayout->removeWidget( last );
    mUi.gridLayout->removeWidget( mUi.addEmailButton );

    QList<PhoneWidgets*>::const_iterator widgetIt = mPhoneWidgets.constBegin();
    for ( ; widgetIt != mPhoneWidgets.constEnd(); ++widgetIt ) {
      mUi.gridLayout->removeWidget( (*widgetIt)->mInput );
      mUi.gridLayout->removeWidget( (*widgetIt)->mType );
    }

    mUi.gridLayout->removeWidget( mUi.phoneLabel );
    mUi.gridLayout->removeWidget( mUi.addPhoneButton );
    mUi.gridLayout->removeWidget( mUi.cancelButton );
    mUi.gridLayout->removeWidget( mUi.saveButton );
    mUi.gridLayout->removeWidget( mUi.collectionSelector );
    mUi.gridLayout->removeWidget( mUi.launchAccountWizardButton );

    // delete the now obsolete widget
    --mLastEmailRow;
    --mLastPhoneRow;
    mEmailInputs.pop_back();
    last->deleteLater();

    // re-add widgets
    mUi.gridLayout->addWidget( mUi.addEmailButton, mLastEmailRow, 2, 1, 1 );

    int row = mLastEmailRow + 1;
    mUi.gridLayout->addWidget( mUi.phoneLabel, row, 0, 1, 1 );
    widgetIt = mPhoneWidgets.constBegin();
    for ( ; widgetIt != mPhoneWidgets.constEnd(); ++widgetIt, ++row ) {
      mUi.gridLayout->addWidget( (*widgetIt)->mInput, row, 1, 1, 1 );
      mUi.gridLayout->addWidget( (*widgetIt)->mType, row, 2, 1, 1 );
    }

    mUi.gridLayout->addWidget( mUi.addPhoneButton, mLastPhoneRow, 3, 1, 1 );
    mUi.gridLayout->addWidget( mUi.cancelButton, row, 2, 1, 1 );
    mUi.gridLayout->addWidget( mUi.saveButton, row, 3, 1, 1 );
    mUi.gridLayout->addWidget( mUi.collectionSelector, row, 1, 1, 1 );
    mUi.gridLayout->addWidget( mUi.launchAccountWizardButton, row, 1, 1, 1 );
  } else {
    last->clear();
  }
}

void EditorGeneral::Private::addPhoneRows( int newRowCount )
{
  if ( newRowCount <= mPhoneWidgets.count() ) {
    return;
  }

  // remove widgets from layout
  mUi.gridLayout->removeWidget( mUi.addPhoneButton );
  mUi.gridLayout->removeWidget( mUi.cancelButton );
  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );
  mUi.gridLayout->removeWidget( mUi.launchAccountWizardButton );

  int row = mLastPhoneRow + 1;
  // add new widgets
  for ( ; mPhoneWidgets.count() < newRowCount; ++row, ++mLastPhoneRow ) {
    MobileLineEdit *lineEdit = new MobileLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    PhoneTypeCombo *combo = new PhoneTypeCombo( q );
    mUi.gridLayout->addWidget( combo, row, 2, 1, 1 );

    QObject::connect( lineEdit, SIGNAL(clearClicked()), q, SLOT(clearPhoneClicked()) );
    mPhoneWidgets << new PhoneWidgets( lineEdit, combo );
  }

  // re-add widgets
  mUi.gridLayout->addWidget( mUi.addPhoneButton, mLastPhoneRow, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.cancelButton, mLastPhoneRow + 1, 2, 1, 1 );
  mUi.gridLayout->addWidget( mUi.saveButton, mLastPhoneRow + 1, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, mLastPhoneRow + 1, 1, 1, 1 );
  mUi.gridLayout->addWidget( mUi.launchAccountWizardButton, mLastPhoneRow + 1, 1, 1, 1 );
}

void EditorGeneral::Private::clearPhoneClicked()
{
  int index = 0;
  for ( ; index < mPhoneWidgets.count(); ++index ) {
    if ( mPhoneWidgets[ index ]->mInput == q->sender() ) {
      break;
    }
  }

  Q_ASSERT( index >= 0 && index < mPhoneWidgets.count() );

  // shift data
  for ( int i = index + 1; i < mPhoneWidgets.count(); ++i ) {
    PhoneWidgets *source = mPhoneWidgets[ i ];
    PhoneWidgets *target = mPhoneWidgets[ i - 1 ];
    target->mInput->setText( source->mInput->text() );
    target->mType->setType( source->mType->type() );
    target->mId = source->mId;
  }

  PhoneWidgets *last = mPhoneWidgets.last();
  if ( mPhoneWidgets.count() > 2 ) {
    // remove widgets from layout
    mUi.gridLayout->removeWidget( mUi.addPhoneButton );
    mUi.gridLayout->removeWidget( mUi.cancelButton );
    mUi.gridLayout->removeWidget( mUi.saveButton );
    mUi.gridLayout->removeWidget( mUi.collectionSelector );
    mUi.gridLayout->removeWidget( mUi.launchAccountWizardButton );

    --mLastPhoneRow;
    mPhoneWidgets.pop_back();
    last->mInput->deleteLater();
    last->mType->deleteLater();
    delete last;

    // re-add widgets
    mUi.gridLayout->addWidget( mUi.addPhoneButton, mLastPhoneRow, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.cancelButton, mLastPhoneRow + 1, 2, 1, 1 );
    mUi.gridLayout->addWidget( mUi.saveButton, mLastPhoneRow + 1, 3, 1, 1 );
    mUi.gridLayout->addWidget( mUi.collectionSelector, mLastPhoneRow + 1, 1, 1, 1 );
    mUi.gridLayout->addWidget( mUi.launchAccountWizardButton, mLastPhoneRow + 1, 1, 1, 1 );
  } else {
    last->mInput->clear();
    last->mType->setType( KContacts::PhoneNumber::Home );
    last->mId.clear();
  }
}

EditorGeneral::EditorGeneral( QWidget *parent )
  : EditorBase( parent ), d( new Private( this ) )
{
  connect( d->mUi.fullName, SIGNAL(textChanged(QString)), SLOT(nameTextChanged(QString)) );

  connect( d->mUi.addEmailButton, SIGNAL(clicked()), SLOT(addEmailClicked()) );

  connect( d->mUi.addPhoneButton, SIGNAL(clicked()), SLOT(addPhoneClicked()) );

  connect( d->mUi.saveButton, SIGNAL(clicked()), SLOT(disableSaveButton()) ); // prevent double clicks
  connect( d->mUi.saveButton, SIGNAL(clicked()), SIGNAL(saveClicked()) );
  connect( d->mUi.cancelButton, SIGNAL(clicked()), SIGNAL(cancelClicked()) );
  connect( d->mUi.collectionSelector, SIGNAL(currentChanged(Akonadi::Collection)),
           SIGNAL(collectionChanged(Akonadi::Collection)) );

  connect( d->mUi.collectionSelector->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
           SLOT(availableCollectionsChanged()) );
  connect( d->mUi.collectionSelector->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
           SLOT(availableCollectionsChanged()) );

  d->mUi.saveButton->setIcon( SmallIcon( QLatin1String("document-save"), 64 ) );
  d->mUi.cancelButton->setIcon( SmallIcon( QLatin1String("dialog-cancel"), 64 ) );
}

EditorGeneral::~EditorGeneral()
{
  delete d;
}

void EditorGeneral::loadContact( const KContacts::Addressee &contact, const Akonadi::ContactMetaData& )
{
  d->mContact = contact;

  d->mUi.fullName->blockSignals( true );
  d->mUi.fullName->setText( contact.assembledName() );
  d->mUi.fullName->blockSignals( false );

  const QStringList emails = contact.emails();
  d->ensureEmailRows( emails.count() );

  QList<MobileLineEdit*>::iterator inputIt = d->mEmailInputs.begin();
  Q_FOREACH( const QString &email, emails ) {
    (*inputIt)->setText( email );
    ++inputIt;
  }

  const KContacts::PhoneNumber::List phones = contact.phoneNumbers();
  d->ensurePhoneRows( phones.count() );

  QList<PhoneWidgets*>::iterator widgetIt = d->mPhoneWidgets.begin();
  Q_FOREACH( const KContacts::PhoneNumber &phone, phones ) {
    PhoneWidgets* widgets = *widgetIt;

    widgets->mId = phone.id();
    widgets->mInput->setText( phone.number() );

    widgets->mType->setType( phone.type() );

    ++widgetIt;
  }

  d->mUi.pictureButton->loadContact( contact );
}

void EditorGeneral::saveContact( KContacts::Addressee &contact, Akonadi::ContactMetaData& ) const
{
  contact.setPrefix( d->mContact.prefix() );
  contact.setGivenName( d->mContact.givenName() );
  contact.setAdditionalName( d->mContact.additionalName() );
  contact.setFamilyName( d->mContact.familyName() );
  contact.setSuffix( d->mContact.suffix() );

  Q_FOREACH( MobileLineEdit *input, d->mEmailInputs ) {
    const QString email = input->text().trimmed();
    if ( !email.isEmpty() ) {
      contact.insertEmail( email );
    }
  }

  Q_FOREACH( PhoneWidgets *widgets, d->mPhoneWidgets ) {
    const QString number = widgets->mInput->text().trimmed();
    if ( !number.isEmpty() ) {
      KContacts::PhoneNumber phone( number, widgets->mType->type() );
      if ( !widgets->mId.isEmpty() ) {
        phone.setId( widgets->mId );
      }
      contact.insertPhoneNumber( phone );
    }
  }

  d->mUi.pictureButton->storeContact( contact );
}

Akonadi::Collection EditorGeneral::selectedCollection() const
{
  return d->mUi.collectionSelector->currentCollection();
}

void EditorGeneral::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mUi.collectionSelector->setDefaultCollection( collection );
}

void EditorGeneral::updateName( const KContacts::Addressee &contact )
{
  // this slot is called when the name parts have been changed in the 'More' page
  blockSignals( true );
  d->mContact = contact;
  d->mUi.fullName->setText( d->mContact.assembledName() );
  d->mUi.saveButton->setEnabled( !d->mUi.fullName->text().trimmed().isEmpty() );
  blockSignals( false );
}

#include "moc_editorgeneral.cpp"
