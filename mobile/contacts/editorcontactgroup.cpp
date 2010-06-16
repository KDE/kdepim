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

#include <KABC/Addressee>
#include <KABC/ContactGroup>

class EditorContactGroup::Private
{
  EditorContactGroup *const q;
 
  public:
    explicit Private( EditorContactGroup *parent )
      : q( parent )
    {
      mUi.setupUi( parent );

      mInputs << mUi.recipient1;
      mInputs << mUi.recipient2;
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

    QList<QLineEdit*> mInputs;

    int mLastRow;

  public: // slots
    void nameTextChanged( const QString &text )
    {
      mUi.saveButton->setEnabled( !text.trimmed().isEmpty() );
    }
   
    void addRecipientClicked();

  private:
    void addRows( int newRowCount );
};

void EditorContactGroup::Private::addRecipientClicked()
{
  addRows( mInputs.count() + 1 );
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
    mInputs << lineEdit;
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
  
  QStringList emails;
  for ( uint i = 0; i < contactGroup.dataCount(); ++i ) {
    const KABC::ContactGroup::Data &data = contactGroup.data( i );
    contact.setNameFromString( data.name() );
    emails << contact.fullEmail( data.email() );
  }
  
  d->ensureRows( emails.count() );

  QList<QLineEdit*>::iterator inputIt = d->mInputs.begin();
  Q_FOREACH( const QString &email, emails ) {
    (*inputIt)->setText( email );
    ++inputIt;
  }  
}

void EditorContactGroup::saveContactGroup( KABC::ContactGroup &contactGroup )
{
  contactGroup.setName(  d->mUi.groupName->text() );
  contactGroup.setId( d->mContactGroup.id() );

  Q_FOREACH( QLineEdit *input, d->mInputs ) {
    const QString email = input->text().trimmed();
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

#include "editorcontactgroup.moc"
