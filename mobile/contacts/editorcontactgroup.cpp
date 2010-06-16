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

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KABC/Addressee>
#include <KABC/ContactGroup>

class Recipient
{
  public:
    Recipient( QLineEdit *input ) : mInput( input ) {}
   
  public:
    QLineEdit* mInput;
    
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

      mInputs << new Recipient( mUi.recipient1 );
      mInputs << new Recipient( mUi.recipient2 );
      mLastRow = 2; // third row
      
      mUi.collectionSelector->setMimeTypeFilter( QStringList() << KABC::ContactGroup::mimeType() );
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
    kError() << "Fetching contact item" << item.id() << "failed:" << fetchJob->errorString();
  } else if ( !item.hasPayload<KABC::Addressee>() ) {
    kError() << "Fetching contact item" << item.id() << "worked but it is not a contact";
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

void EditorContactGroup::Private::addRows( int newRowCount )
{
  if ( newRowCount <= mInputs.count() ) {
    return;
  }

  // remove widgets from layout
  mUi.gridLayout->removeWidget( mUi.addRecipientButton );

  mUi.gridLayout->removeWidget( mUi.saveButton );
  mUi.gridLayout->removeWidget( mUi.collectionSelector );

  int row = mLastRow + 1;
  
  // add new widgets
  for ( ; mInputs.count() < newRowCount; ++row, ++mLastRow ) {
    QLineEdit *lineEdit = new QLineEdit( q );
    mUi.gridLayout->addWidget( lineEdit, row, 1, 1, 1 );
    mInputs << new Recipient( lineEdit );
  }

  // re-add widgets
  mUi.gridLayout->addWidget( mUi.addRecipientButton, mLastRow, 2, 1, 1 );

  mUi.gridLayout->addWidget( mUi.saveButton, row, 1, 1, 1 );
  mUi.gridLayout->addWidget( mUi.collectionSelector, row, 2, 1, 1 );
}

EditorContactGroup::EditorContactGroup( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  connect( d->mUi.addRecipientButton, SIGNAL( clicked() ), SLOT( addRecipientClicked() ) );
  
  connect( d->mUi.saveButton, SIGNAL( clicked() ), SIGNAL( saveClicked() ) );
  connect( d->mUi.collectionSelector, SIGNAL( currentChanged( Akonadi::Collection ) ),
           SIGNAL( collectionChanged( Akonadi::Collection ) ) );
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

  for ( uint i = 0; inputIt != d->mInputs.constEnd(); ++inputIt, ++i, ++count ) {
    const KABC::ContactGroup::ContactReference &ref = contactGroup.contactReference( i );
    (*inputIt)->mItem.setId( ref.uid().toLongLong() );
    (*inputIt)->mPreferredEmail = ref.preferredEmail();
    (*inputIt)->mInput->setText( i18nc( "@info:status", "Loading..." ) );
    (*inputIt)->mInput->setEnabled( false );

    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( (*inputIt)->mItem );
    job->fetchScope().fetchFullPayload( true );
    job->setProperty( "RecipientIndex", count );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( fetchResult( KJob* ) ) );
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

      if ( input->mItem.isValid() ) {
        const Akonadi::Item item = input->mItem;
        KABC::ContactGroup::ContactReference ref;
        ref.setUid( QString::number( item.id() ) );

        if ( !emailPart.isEmpty() ) {
          if ( !item.hasPayload<KABC::Addressee>() || item.payload<KABC::Addressee>().preferredEmail() != emailPart ) {
            ref.setPreferredEmail( emailPart );
          }
        }

        contactGroup.append( ref );
      } else if ( !emailPart.isEmpty() ) {
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

#include "editorcontactgroup.moc"
