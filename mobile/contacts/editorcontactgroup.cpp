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

#include "editorcontactgroup.h"

#include "ui_editorcontactgroup.h"

#include "contactcompletionmodel_p.h"

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <KABC/kabc/Addressee>
#include <KABC/kabc/ContactGroup>

#include <KLocalizedString>
#include <QDebug>
#include <KIconLoader>

#include <QCompleter>
#include <QSortFilterProxyModel>

class Recipient
{
  public:
    Recipient( MobileLineEdit *input, QObject *receiver )
      : mInput( input )
    {
      QObject::connect( mInput, SIGNAL(clearClicked()), receiver, SLOT(clearRecipientClicked()) );
    }

  public:
    MobileLineEdit* mInput;

    Akonadi::Item mItem;
    QString mPreferredEmail;
};

class EditorContactGroup::Private
{
  EditorContactGroup *const q;

  public:
    explicit Private( EditorContactGroup *parent )
      : q( parent )
    {
      mUi.setupUi( parent );
      mUi.cancelButton->setIcon( SmallIcon( QLatin1String("dialog-cancel"), 64 ) );
      mUi.saveButton->setIcon( SmallIcon( QLatin1String("document-save"), 64 ) );
      mUi.saveButton->setEnabled( false );

      mInputs << new Recipient( mUi.recipient1, q );
      mInputs << new Recipient( mUi.recipient2, q );
      mLastRow = 2; // third row

      mUi.collectionSelector->setMimeTypeFilter( QStringList() << KABC::ContactGroup::mimeType() );
      mUi.collectionSelector->setAccessRightsFilter( Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanChangeItem );

      QObject::connect( mUi.launchAccountWizardButton, SIGNAL(clicked()), q, SIGNAL(requestLaunchAccountWizard()) );
      QObject::connect( mUi.groupName, SIGNAL(textChanged(QString)), q, SLOT(nameTextChanged(QString)) );

      availableCollectionsChanged();
    }

    void ensureRows( int recipientCount )
    {
      // TODO delete unnecessary rows?
      addRows( recipientCount );
    }

  public:
    Ui::EditorContactGroup mUi;

    KABC::ContactGroup mContactGroup;

    QList<Recipient*> mInputs;

    int mLastRow;

  public: // slots
    void nameTextChanged( const QString &text )
    {
      mUi.saveButton->setEnabled( !text.trimmed().isEmpty() );
    }

    void addRecipientClicked();

    void fetchResult( KJob *job );

    void clearRecipientClicked();

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
    void addRows( int newRowCount );
};

void EditorContactGroup::Private::addRecipientClicked()
{
  addRows( mInputs.count() + 1 );
}

void EditorContactGroup::Private::fetchResult( KJob *job )
{
  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
  Q_ASSERT( fetchJob != 0 );

  int index = fetchJob->property( "RecipientIndex" ).value<int>();
  Q_ASSERT( index >= 0 && index < mInputs.count() );
  Recipient *recipient = mInputs[ index ];

  const Akonadi::Item item = fetchJob->items().isEmpty() ? recipient->mItem : fetchJob->items().first();
  if ( fetchJob->error() != 0 ) {
    qCritical() << "Fetching contact item" << item.id() << "failed:" << fetchJob->errorString();
  } else if ( !item.hasPayload<KABC::Addressee>() ) {
    qCritical() << "Fetching contact item" << item.id() << "worked but it is not a contact";
  } else {
    const KABC::Addressee contact = item.payload<KABC::Addressee>();

    recipient->mItem = item;
    recipient->mInput->setEnabled( true );

    if ( recipient->mPreferredEmail.isEmpty() ) {
      recipient->mInput->setText( contact.fullEmail( contact.preferredEmail() ) );
    } else {
      recipient->mInput->setText( contact.fullEmail( recipient->mPreferredEmail ) );
    }
  }
}

void EditorContactGroup::Private::clearRecipientClicked()
{
  int index = 0;
  for ( ; index < mInputs.count(); ++index ) {
    if ( mInputs[ index ]->mInput == q->sender() ) {
      break;
    }
  }

  Q_ASSERT( index >= 0 && index < mInputs.count() );

  // shift data
  for ( int i = index + 1; i < mInputs.count(); ++i ) {
    Recipient *source = mInputs[ i ];
    Recipient *target = mInputs[ i - 1 ];
    target->mInput->setText( source->mInput->text() );
    target->mItem = source->mItem;
    target->mPreferredEmail = source->mPreferredEmail;
  }

  Recipient *last = mInputs.last();
  if ( mInputs.count() > 2 ) {
    // remove widgets from layout
    mUi.gridLayout->removeWidget( mUi.addRecipientButton );
    mUi.gridLayout->removeWidget( mUi.cancelButton );
    mUi.gridLayout->removeWidget( mUi.saveButton );
    mUi.gridLayout->removeWidget( mUi.collectionSelector );

    const int row = mLastRow;
    --mLastRow;
    mInputs.pop_back();
    last->mInput->deleteLater();
    delete last;

    // re-add widgets
    mUi.gridLayout->addWidget( mUi.addRecipientButton, mLastRow, 2, 1, 2 );
    mUi.gridLayout->addWidget( mUi.cancelButton, row, 2, 1, 1 );
    mUi.gridLayout->addWidget( mUi.saveButton, row, 3, 1, 1 );
    mUi.gridLayout->addWidget( mUi.collectionSelector, row, 1, 1, 1 );
  } else {
    last->mInput->clear();
    last->mItem = Akonadi::Item();
    last->mPreferredEmail.clear();
  }
}

void EditorContactGroup::Private::addRows( int newRowCount )
{
  if ( newRowCount <= mInputs.count() ) {
    return;
  }

  // remove widgets from layout
  mUi.gridLayout->removeWidget( mUi.addRecipientButton );
  mUi.gridLayout->removeWidget( mUi.cancelButton );
  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );

  int row = mLastRow + 1;

  // add new widgets
  for ( ; mInputs.count() < newRowCount; ++row, ++mLastRow ) {
    MobileLineEdit *lineEdit = new MobileLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    mInputs << new Recipient( lineEdit, q );
  }

  // re-add widgets
  mUi.gridLayout->addWidget( mUi.addRecipientButton, mLastRow, 2, 1, 2 );
  mUi.gridLayout->addWidget( mUi.cancelButton, row, 2, 1, 1 );
  mUi.gridLayout->addWidget( mUi.saveButton, row, 3, 1, 1 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, row, 1, 1, 1 );
}

EditorContactGroup::EditorContactGroup( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  connect( d->mUi.addRecipientButton, SIGNAL(clicked()), SLOT(addRecipientClicked()) );

  connect( d->mUi.cancelButton, SIGNAL(clicked()), SIGNAL(cancelClicked()) );
  connect( d->mUi.saveButton, SIGNAL(clicked()), SLOT(disableSaveButton()) ); // prevent double clicks
  connect( d->mUi.saveButton, SIGNAL(clicked()), SIGNAL(saveClicked()) );
  connect( d->mUi.collectionSelector, SIGNAL(currentChanged(Akonadi::Collection)),
           SIGNAL(collectionChanged(Akonadi::Collection)) );

  connect( d->mUi.collectionSelector->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
           SLOT(availableCollectionsChanged()) );
  connect( d->mUi.collectionSelector->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
           SLOT(availableCollectionsChanged()) );
}

EditorContactGroup::~EditorContactGroup()
{
  delete d;
}

void EditorContactGroup::loadContactGroup( const KABC::ContactGroup &contactGroup )
{
  d->mContactGroup = contactGroup;

  d->mUi.groupName->setText( contactGroup.name() );

  KABC::Addressee contact;

  d->ensureRows( contactGroup.dataCount() + contactGroup.contactReferenceCount() );

  int count = 0;

  QStringList emails;
  for ( uint i = 0; i < contactGroup.dataCount(); ++i, ++count ) {
    const KABC::ContactGroup::Data &data = contactGroup.data( i );
    contact.setNameFromString( data.name() );
    emails << contact.fullEmail( data.email() );
  }

  QList<Recipient*>::const_iterator inputIt = d->mInputs.constBegin();
  Q_FOREACH( const QString &email, emails ) {
    (*inputIt)->mInput->setText( email );
    ++inputIt;
  }

  for ( uint i = 0; inputIt != d->mInputs.constEnd() && i < contactGroup.contactReferenceCount(); ++inputIt, ++i, ++count ) {
    const KABC::ContactGroup::ContactReference &ref = contactGroup.contactReference( i );
    (*inputIt)->mItem.setId( ref.uid().toLongLong() );
    (*inputIt)->mPreferredEmail = ref.preferredEmail();
    (*inputIt)->mInput->setText( i18nc( "@info:status", "Loading..." ) );
    (*inputIt)->mInput->setEnabled( false );

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( (*inputIt)->mItem );
    job->fetchScope().fetchFullPayload( true );
    job->setProperty( "RecipientIndex", count );
    connect( job, SIGNAL(result(KJob*)), SLOT(fetchResult(KJob*)) );
  }
}

void EditorContactGroup::saveContactGroup( KABC::ContactGroup &contactGroup ) const
{
  contactGroup.setName(  d->mUi.groupName->text() );
  contactGroup.setId( d->mContactGroup.id() );

  Q_FOREACH( Recipient *input, d->mInputs ) {
    const QString email = input->mInput->text().trimmed();
    if ( !email.isEmpty() ) {
      QString namePart;
      QString emailPart;
      KABC::Addressee::parseEmailAddress( email, namePart, emailPart );
      if ( namePart.isEmpty() ) {
        namePart = emailPart;
      }

      if ( !emailPart.isEmpty() ) {
        contactGroup.append( KABC::ContactGroup::Data( namePart, emailPart ) );
      }
    }
  }
}

Akonadi::Collection EditorContactGroup::selectedCollection() const
{
  return d->mUi.collectionSelector->currentCollection();
}

void EditorContactGroup::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mUi.collectionSelector->setDefaultCollection( collection );
}

#include "moc_editorcontactgroup.cpp"
