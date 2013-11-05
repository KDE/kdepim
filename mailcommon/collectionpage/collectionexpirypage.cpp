/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 2009 Montel Laurent <montel@kde.org>
  Copyright (c) 2013 Jonathan Marten <jjm@keelhaul.me.uk>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "collectionexpirypage.h"

#include "expirecollectionattribute.h"
#include "folderrequester.h"
#include "foldercollection.h"
#include "util/mailutil.h"
#include "kernel/mailkernel.h"

#include <Akonadi/CollectionModifyJob>

#include <KDialog>
#include <KIntSpinBox>
#include <KLocale>
#include <KMessageBox>

#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

using namespace Akonadi;
using namespace MailCommon;

CollectionExpiryPage::CollectionExpiryPage( QWidget *parent )
  : CollectionPropertiesPage( parent )
{
  setObjectName( QLatin1String( "MailCommon::CollectionExpiryPage" ) );
  setPageTitle( i18nc( "@title:tab Expiry settings for a folder.", "Expiry" ) );
}

CollectionExpiryPage::~CollectionExpiryPage()
{
}

bool CollectionExpiryPage::canHandle( const Akonadi::Collection &col ) const
{
  QSharedPointer<FolderCollection> fd = FolderCollection::forCollection( col, false );
  return ( fd->canDeleteMessages() && !fd->isStructural() );
}

void CollectionExpiryPage::init()
{
  QVBoxLayout *globalVBox = new QVBoxLayout( this );
  globalVBox->setSpacing( KDialog::spacingHint() );

  QGridLayout *daysBox = new QGridLayout;

  expireReadMailCB = new QCheckBox;
  expireReadMailCB->setText( i18n( "Expire read messages after" ) );
  connect( expireReadMailCB, SIGNAL(toggled(bool)),
           this, SLOT(slotUpdateControls()) );
  daysBox->addWidget( expireReadMailCB, 0, 0, Qt::AlignLeft );

  expireReadMailSB = new KIntSpinBox;
  expireReadMailSB->setMaximum( 999999 );
  expireReadMailSB->setValue( 30 );
  expireReadMailSB->setSuffix( ki18ncp("Expire messages after %1", " day", " days" ) );
  daysBox->addWidget( expireReadMailSB, 0, 1 );
  connect(expireReadMailSB,SIGNAL(valueChanged(int)),SLOT(slotChanged()));

  expireUnreadMailCB = new QCheckBox;
  expireUnreadMailCB->setText( i18n( "Expire unread messages after" ) );
  connect( expireUnreadMailCB, SIGNAL(toggled(bool)),
           this, SLOT(slotUpdateControls()) );
  daysBox->addWidget( expireUnreadMailCB, 1, 0, Qt::AlignLeft );

  expireUnreadMailSB = new KIntSpinBox;
  expireUnreadMailSB->setMaximum( 99999 );
  expireUnreadMailSB->setValue( 30 );
  expireUnreadMailSB->setSuffix( ki18ncp("Expire messages after %1", " day", " days" ) );
  daysBox->addWidget( expireUnreadMailSB, 1, 1 );
  connect(expireUnreadMailSB,SIGNAL(valueChanged(int)),SLOT(slotChanged()));

  daysBox->setColumnStretch( 3, 1 );
  globalVBox->addLayout( daysBox );

  globalVBox->addSpacing( 30 );

  QGroupBox *actionsGroup = new QGroupBox;
  actionsGroup->hide(); // for mutual exclusion of the radio buttons

  QHBoxLayout *moveToHBox = new QHBoxLayout();
  moveToHBox->setMargin( 0 );
  moveToHBox->setSpacing( 6 );

  moveToRB = new QRadioButton( actionsGroup );
  moveToRB->setText( i18n( "Move expired messages to:" ) );
  connect( moveToRB, SIGNAL(toggled(bool)), this, SLOT(slotUpdateControls()) );
  moveToHBox->addWidget( moveToRB );

  folderSelector = new FolderRequester( this );
  folderSelector->setMustBeReadWrite( true );
  folderSelector->setShowOutbox( false );
  moveToHBox->addWidget( folderSelector );
  globalVBox->addLayout( moveToHBox );
  connect(folderSelector,SIGNAL(folderChanged(Akonadi::Collection)),SLOT(slotChanged()));

  deletePermanentlyRB = new QRadioButton( actionsGroup );
  deletePermanentlyRB->setText( i18n( "Delete expired messages permanently" ) );
  connect(deletePermanentlyRB, SIGNAL(toggled(bool)), this, SLOT(slotUpdateControls()) );

  globalVBox->addWidget( deletePermanentlyRB );

  globalVBox->addSpacing( 30 );

  expireNowPB = new QPushButton( i18n( "Save Settings and Expire Now" ), this );
  connect(expireNowPB, SIGNAL(clicked()), SLOT(slotSaveAndExpire()));
  globalVBox->addWidget( expireNowPB, 0, Qt::AlignRight );

  globalVBox->addStretch( 100 ); // eat all superfluous space
}

void CollectionExpiryPage::load( const Akonadi::Collection &collection )
{
  mCollection = collection;
  init();

  bool mustDeleteExpirationAttribute = false;
  MailCommon::ExpireCollectionAttribute *attr = MailCommon::ExpireCollectionAttribute::expirationCollectionAttribute( mCollection, mustDeleteExpirationAttribute );

  // Load the values from the folder
  bool expiryGloballyOn = attr->isAutoExpire();
  int daysToExpireRead, daysToExpireUnread;
  attr->daysToExpire( daysToExpireUnread, daysToExpireRead);

  if ( expiryGloballyOn
       && attr->readExpireUnits() != ExpireCollectionAttribute::ExpireNever
       && daysToExpireRead >= 0 ) {
    expireReadMailCB->setChecked( true );
    expireReadMailSB->setValue( daysToExpireRead );
  }
  if ( expiryGloballyOn
      && attr->unreadExpireUnits() != ExpireCollectionAttribute::ExpireNever
      && daysToExpireUnread >= 0 ) {
    expireUnreadMailCB->setChecked( true );
    expireUnreadMailSB->setValue( daysToExpireUnread );
  }

  if ( attr->expireAction() == ExpireCollectionAttribute::ExpireDelete )
    deletePermanentlyRB->setChecked( true );
  else
    moveToRB->setChecked( true );

  Akonadi::Collection::Id destFolderID = attr->expireToFolderId();
  if ( destFolderID > 0 ) {
    Akonadi::Collection destFolder = Kernel::self()->collectionFromId( destFolderID );
    if ( destFolder.isValid() )
      folderSelector->setCollection( destFolder );
  }

  if ( mustDeleteExpirationAttribute )
    delete attr;
  slotUpdateControls();
  mChanged = false;
}

void CollectionExpiryPage::save( Akonadi::Collection &collection )
{
  if ( mChanged)
    saveAndExpire( collection, false, true );
}

void CollectionExpiryPage::saveAndExpire( Akonadi::Collection &collection, bool saveSettings, bool _expireNow )
{
  bool expireNow = _expireNow;
  bool enableGlobally = expireReadMailCB->isChecked() || expireUnreadMailCB->isChecked();
  const Akonadi::Collection expireToFolder = folderSelector->collection();
  if ( enableGlobally && moveToRB->isChecked() && !expireToFolder.isValid() ) {
    KMessageBox::error( this, i18n("Please select a folder to expire messages into.\nIf this is not done, expired messages will be permanently deleted."),
                        i18n( "No Folder Selected" ) );
    deletePermanentlyRB->setChecked( true );
    expireNow = false;                                // settings are not valid
  }

  MailCommon::ExpireCollectionAttribute *attribute = 0;
  if ( expireToFolder.isValid() && moveToRB->isChecked() ) {
    if ( expireToFolder.id() == collection.id() ) {
      KMessageBox::error( this, i18n( "Please select a different folder than the current folder to expire messages into.\nIf this is not done, expired messages will be permanently deleted."),
                          i18n( "Wrong Folder Selected" ) );
      deletePermanentlyRB->setChecked( true );
      expireNow = false;                                // settings are not valid
    }
    else {
      attribute = collection.attribute<MailCommon::ExpireCollectionAttribute>( Akonadi::Entity::AddIfMissing );
      attribute->setExpireToFolderId( expireToFolder.id() );
    }
  }
  if ( !attribute )
    attribute =  collection.attribute<MailCommon::ExpireCollectionAttribute>( Akonadi::Entity::AddIfMissing );

  attribute->setAutoExpire( enableGlobally );
  // we always write out days now
  attribute->setReadExpireAge( expireReadMailSB->value() );
  attribute->setUnreadExpireAge( expireUnreadMailSB->value() );
  attribute->setReadExpireUnits( expireReadMailCB->isChecked() ? MailCommon::ExpireCollectionAttribute::ExpireDays :
                                 MailCommon::ExpireCollectionAttribute::ExpireNever );
  attribute->setUnreadExpireUnits( expireUnreadMailCB->isChecked() ? MailCommon::ExpireCollectionAttribute::ExpireDays :
                                   MailCommon::ExpireCollectionAttribute::ExpireNever );

  if ( deletePermanentlyRB->isChecked() )
    attribute->setExpireAction( ExpireCollectionAttribute::ExpireDelete );
  else
    attribute->setExpireAction( ExpireCollectionAttribute::ExpireMove );
  if (saveSettings) {
      Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob( collection, this );
      job->setProperty( "expireNow", expireNow );
      connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCollectionModified(KJob*)) );
  } else {
      if (expireNow) {
          MailCommon::Util::expireOldMessages( collection, true /*immediate*/);
      }
  }
  mChanged = false;
}

void CollectionExpiryPage::slotSaveAndExpire()
{
  saveAndExpire( mCollection, true, true );                        // save and start expire job
}

void CollectionExpiryPage::slotCollectionModified(KJob* job)
{
  if ( job->error() ) {
    kDebug()<<" Error when we modified collection";
    return;
  }

  // trigger immediate expiry if there is something to do
  if ( job->property( "expireNow" ).toBool() )
    MailCommon::Util::expireOldMessages( mCollection, true /*immediate*/);
}

void CollectionExpiryPage::slotUpdateControls()
{
  const bool showExpiryActions = expireReadMailCB->isChecked() || expireUnreadMailCB->isChecked();
  moveToRB->setEnabled( showExpiryActions );
  folderSelector->setEnabled( showExpiryActions && moveToRB->isChecked() );
  deletePermanentlyRB->setEnabled( showExpiryActions );

  expireReadMailSB->setEnabled( expireReadMailCB->isChecked() );
  expireUnreadMailSB->setEnabled( expireUnreadMailCB->isChecked() );

  expireNowPB->setEnabled( showExpiryActions );

  mChanged = true;
}

void CollectionExpiryPage::slotChanged()
{
  mChanged = true;
}

