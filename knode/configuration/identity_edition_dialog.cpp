/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "identity_edition_dialog.h"

#include "knglobals.h"

#include <KDebug>
#include <KMessageBox>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>

using namespace KPIMIdentities;

namespace KNode {


IdentityEditionDialog::IdentityEditionDialog( uint uoid, QWidget *parent )
  : KDialog( parent ),
    mCurrentIdentityUoid( -1 )
{
  setupUi( this );

  setCaption( i18nc( "@title:window", "Manage your identities" ) );

  mSigningKeyRequester->dialogButton()->setText( i18nc( "@action:button Change signing key", "Change..." ) );
  mSigningKeyRequester->setDialogCaption( i18nc( "@title:window PGP key chooser", "Your OpenPGP Key") );
  mSigningKeyRequester->setDialogMessage( i18n( "Select the OpenPGP key which should be "
                                                "used for signing articles." ) );
  mSigningKeyRequester->setAllowedKeys( Kleo::SigningKeyRequester::OpenPGP );

  mButtonNewIdentity->setIcon( KIcon( "list-add" ) );
  mButtonDuplicateIdentity->setIcon( KIcon( "edit-copy" ) );
  mButtonRenameIdentity->setIcon( KIcon( "edit-rename" ) );
  mButtonRemoveIdentity->setIcon( KIcon( "edit-delete" ) );

  setMainWidget( page );


  connect( mIdentitySelector, SIGNAL(currentIndexChanged(int)),
           this, SLOT(identitySelected(int)) );

  connect( mButtonNewIdentity, SIGNAL(clicked(bool)),
           this, SLOT(createNewIdentity()) );
  connect( mButtonDuplicateIdentity, SIGNAL(clicked(bool)),
           this, SLOT(duplicateCurrentIdentity()) );
  connect( mButtonRenameIdentity, SIGNAL(clicked(bool)),
           this, SLOT(startIdentityRenaming()) );
  connect( mButtonRemoveIdentity, SIGNAL(clicked(bool)),
           this, SLOT(deleteCurrentIdentity()) );


  reload();
  setCurrentIdentity( uoid );
}

IdentityEditionDialog::~IdentityEditionDialog()
{
}


void IdentityEditionDialog::reload()
{
  IdentityManager *im = KNGlobals::self()->identityManager();

  IdentityManager::Iterator it = im->modifyBegin();
  IdentityManager::Iterator end = im->modifyEnd();
  mUoids.clear();
  mIdentitySelector->blockSignals( true ); // We don't want to call identitySelected()
  mIdentitySelector->clear();
  while ( it != end ) {
    mUoids << (*it).uoid();
    mIdentitySelector->addItem( (*it).identityName(), (*it).uoid() );
    ++it;
  }
  mIdentitySelector->blockSignals( false );

  bool canDelIdentity = ( mUoids.size() > 1 );
  mButtonRemoveIdentity->setEnabled( canDelIdentity );
  mIdentitySelector->setEditable( false );
}


void IdentityEditionDialog::slotButtonClicked( int button )
{
  IdentityManager *im = KNGlobals::self()->identityManager();

  switch ( button ) {

    case KDialog::Ok:
    case KDialog::Apply:
      if ( mCurrentIdentityUoid != -1 ) {
        saveIntoIdentity( mCurrentIdentityUoid );
      }
      im->commit();
      break;

    case KDialog::Cancel:
    case KDialog::Close:
      im->rollback();
      break;
  }

  KDialog::slotButtonClicked( button );
}


void IdentityEditionDialog::identitySelected( int index )
{
  if ( index < 0 || index >= mUoids.size() ) {
    kWarning() << "Bad state: called with the index" << index << "when mUoids.size()==" << mUoids.size();
    return;
  }

  setCurrentIdentity( mUoids[ index ] );
}


void IdentityEditionDialog::loadFromIdentity( uint uoid )
{
  IdentityManager *im = KNGlobals::self()->identityManager();

  Identity identity = im->modifyIdentityForUoid( uoid );

  mNameEdit->setText( identity.fullName() );
  mOrganisationEdit->setText( identity.organization() );
  mEmailEdit->setText( identity.primaryEmailAddress() );
  mReplytoEdit->setText( identity.replyToAddr() );
  mMailcopiestoEdit->setText( identity.property( "Mail-Copies-To" ).toString() );
  mSignatureConfigurator->setSignature( identity.signature() );
  mSigningKeyRequester->setFingerprint( QString::fromLatin1( identity.pgpSigningKey() ) );
  mSigningKeyRequester->setInitialQuery( identity.primaryEmailAddress() );
}

void IdentityEditionDialog::saveIntoIdentity( uint uoid ) const
{
  IdentityManager *im = KNGlobals::self()->identityManager();
  Identity &identity = im->modifyIdentityForUoid( uoid );

  identity.setFullName( mNameEdit->text().trimmed() );
  identity.setOrganization( mOrganisationEdit->text().trimmed() );
  identity.setPrimaryEmailAddress( mEmailEdit->text().trimmed() );
  identity.setReplyToAddr( mReplytoEdit->text().trimmed() );
  identity.setProperty( "Mail-Copies-To", mMailcopiestoEdit->text().trimmed() );
  identity.setSignature( mSignatureConfigurator->signature() );
  identity.setPGPSigningKey( mSigningKeyRequester->fingerprint().toLatin1() );
}

void IdentityEditionDialog::setCurrentIdentity( uint uoid )
{
  stopIdentityRenaming();
  if ( mCurrentIdentityUoid != -1 ) {
    saveIntoIdentity( mCurrentIdentityUoid );
  }

  int index = mUoids.indexOf( uoid );
  if ( index == -1 ) {
    index = 0;
  }

  mCurrentIdentityUoid = mUoids[ index ];
  mIdentitySelector->blockSignals( true ); // We don't want to call identitySelected()
  mIdentitySelector->setCurrentIndex( index );
  mIdentitySelector->blockSignals( false );
  loadFromIdentity( mCurrentIdentityUoid );
}

void IdentityEditionDialog::createNewIdentity()
{
  IdentityManager *im = KNGlobals::self()->identityManager();
  const QString idName = im->makeUnique( i18nc( "Name of a newly created identity", "New identity" ) );
  Identity &identity = im->newFromScratch( idName );

  reload();
  setCurrentIdentity( identity.uoid() );
  startIdentityRenaming();
}

void IdentityEditionDialog::duplicateCurrentIdentity()
{
  IdentityManager *im = KNGlobals::self()->identityManager();
  const Identity &currentIdentity = im->modifyIdentityForUoid( mCurrentIdentityUoid );
  const QString newName = im->makeUnique( currentIdentity.identityName() );
  const Identity &newIdentity = im->newFromExisting( currentIdentity, newName );

  reload();
  setCurrentIdentity( newIdentity.uoid() );
  startIdentityRenaming();
}

void IdentityEditionDialog::startIdentityRenaming()
{
  if ( !mIdentitySelector->isEditable() ) {
    mIdentitySelector->setEditable( true );
    if ( !mIdentityNameEdit ) {
      mIdentityNameEdit = new IdentityNameEditPrivate();
      mIdentitySelector->setLineEdit( mIdentityNameEdit );
      connect( mIdentityNameEdit, SIGNAL(identityNameChanged(QString)),
               this, SLOT(changeIdentityName(QString)) );
    }
    mIdentitySelector->setTrapReturnKey( true );
    mIdentitySelector->lineEdit()->selectAll();
    mIdentitySelector->lineEdit()->setFocus( Qt::OtherFocusReason );
  }
}

void IdentityEditionDialog::stopIdentityRenaming()
{
  if ( mIdentitySelector->isEditable() ) {
    mIdentitySelector->setEditable( false );
  }
}

void IdentityEditionDialog::changeIdentityName( const QString &newName )
{
    IdentityManager *im = KNGlobals::self()->identityManager();
    Identity &identity = im->modifyIdentityForUoid( mCurrentIdentityUoid );
    kDebug() << "Change identity name from" << identity.identityName() << "to" << newName;
    Q_ASSERT( !identity.isNull() );
    identity.setIdentityName( newName );

    stopIdentityRenaming();

    reload();
    setCurrentIdentity( identity.uoid() );

    mNameEdit->setFocus( Qt::OtherFocusReason );
}


void IdentityEditionDialog::deleteCurrentIdentity()
{
  if ( mUoids.size() <= 1 ) {
    kWarning() << "Only one identity left and deleteCurrentIdentity() was called!";
    return;
  }

  IdentityManager *im = KNGlobals::self()->identityManager();
  const Identity identity = im->modifyIdentityForUoid( mUoids[ mIdentitySelector->currentIndex() ] );
  int answer = KMessageBox::questionYesNo( this,
                                           i18n( "<qt>Do you really want to remove the identity <emphasis>%1</emphasis>?</qt>",
                                                 identity.identityName() ),
                                           i18nc( "@title:window", "Delete identity" ) );
  if ( answer == KMessageBox::Yes ) {
    mCurrentIdentityUoid = -1;
    im->removeIdentity( identity.identityName() );

    reload();
    setCurrentIdentity( mUoids[ 0 ] );
  }
}

} // namespace KNode

