
#include "expirypropertiesdialog.h"
#include "expirationcollectionattribute.h"
#include "folderrequester.h"
#include "foldercollection.h"
#include "mailutil.h"
#include "mailkernel.h"

#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KIntSpinBox>
#include <klocale.h>
#include <kmessagebox.h>
#include <akonadi/collection.h>
#include <akonadi/collectionmodifyjob.h>

using namespace MailCommon;

/*
 *  Constructs a ExpiryPropertiesDialog as a child of 'parent', with the
 *  name 'name'.
 *
 */
ExpiryPropertiesDialog::ExpiryPropertiesDialog(
  QWidget *tree,
  const Akonadi::Collection &folder )
    : KDialog( tree ),
      mFolder( folder )
{
  setCaption( i18n( "Mail Expiry Properties" ) );
  setButtons( Ok|Cancel );
  setModal( false );
  setObjectName( "expiry_properties" );
  setAttribute( Qt::WA_DeleteOnClose );

  QWidget* privateLayoutWidget = new QWidget;
  privateLayoutWidget->setObjectName( "privateLayoutWidget" );
  setMainWidget( privateLayoutWidget );

  QVBoxLayout *globalVBox = new QVBoxLayout;
  globalVBox->setMargin( marginHint() );
  globalVBox->setObjectName( "globalVBox" );
  globalVBox->setSpacing( spacingHint() );
  privateLayoutWidget->setLayout( globalVBox );

  QGridLayout *daysBox = new QGridLayout;

  expireReadMailCB = new QCheckBox;
  expireReadMailCB->setObjectName( "expireReadMailCB" );
  expireReadMailCB->setText( i18n( "Expire read messages after" ) );
  connect( expireReadMailCB, SIGNAL(toggled(bool)),
           this, SLOT(slotUpdateControls()) );
  daysBox->addWidget( expireReadMailCB, 0, 0, Qt::AlignLeft );

  expireReadMailSB = new KIntSpinBox;
  expireReadMailSB->setObjectName( "expireReadMailSB" );
  expireReadMailSB->setMaximum( 999999 );
  expireReadMailSB->setValue( 30 );
  expireReadMailSB->setSuffix( ki18ncp("Expire messages after %1", " day", " days" ) );
  daysBox->addWidget( expireReadMailSB, 0, 1 );

  expireUnreadMailCB = new QCheckBox;
  expireUnreadMailCB->setObjectName( "expireUnreadMailCB" );
  expireUnreadMailCB->setText( i18n( "Expire unread messages after" ) );
  connect( expireUnreadMailCB, SIGNAL(toggled(bool)),
           this, SLOT(slotUpdateControls()) );
  daysBox->addWidget( expireUnreadMailCB, 1, 0, Qt::AlignLeft );

  expireUnreadMailSB = new KIntSpinBox;
  expireUnreadMailSB->setObjectName( "expireUnreadMailSB" );
  expireUnreadMailSB->setMaximum( 99999 );
  expireUnreadMailSB->setValue( 30 );
  expireUnreadMailSB->setSuffix( ki18ncp("Expire messages after %1", " day", " days" ) );
  daysBox->addWidget( expireUnreadMailSB, 1, 1 );

  daysBox->setColumnStretch( 3, 1 );
  globalVBox->addLayout( daysBox );

  globalVBox->addSpacing( 30 );

  QGroupBox *actionsGroup = new QGroupBox;
  actionsGroup->hide(); // for mutual exclusion of the radio buttons

  QHBoxLayout *moveToHBox = new QHBoxLayout();
  moveToHBox->setMargin( 0 );
  moveToHBox->setSpacing( 6 );

  moveToRB = new QRadioButton( actionsGroup );
  moveToRB->setObjectName( "moveToRB" );
  moveToRB->setText( i18n( "Move expired messages to:" ) );
  connect( moveToRB, SIGNAL(toggled(bool)), this, SLOT(slotUpdateControls()) );
  moveToHBox->addWidget( moveToRB );

  folderSelector = new FolderRequester( privateLayoutWidget );
  folderSelector->setMustBeReadWrite( true );
  folderSelector->setShowOutbox( false );
  moveToHBox->addWidget( folderSelector );
  globalVBox->addLayout( moveToHBox );

  deletePermanentlyRB = new QRadioButton( actionsGroup );
  deletePermanentlyRB->setObjectName( "deletePermanentlyRB" );
  deletePermanentlyRB->setText( i18n( "Delete expired messages permanently" ) );
  globalVBox->addWidget( deletePermanentlyRB );

  globalVBox->addSpacing( 30 );

  QLabel *note = new QLabel;
  note->setObjectName( "note" );
  note->setText( i18n( "Note: Expiry action will be applied immediately after confirming settings." ) );
  note->setAlignment( Qt::AlignVCenter );
  note->setWordWrap( true );

  globalVBox->addWidget( note );
  bool mustDeleteExpirationAttribute = false;
  MailCommon::ExpirationCollectionAttribute *attr = MailCommon::ExpirationCollectionAttribute::expirationCollectionAttribute( mFolder, mustDeleteExpirationAttribute );

  // Load the values from the folder
  bool expiryGloballyOn = attr->isAutoExpire();
  int daysToExpireRead, daysToExpireUnread;
  attr->daysToExpire( daysToExpireUnread, daysToExpireRead);

  if ( expiryGloballyOn
       && attr->readExpireUnits() != ExpirationCollectionAttribute::ExpireNever
       && daysToExpireRead >= 0 ) {
    expireReadMailCB->setChecked( true );
    expireReadMailSB->setValue( daysToExpireRead );
  }
  if ( expiryGloballyOn
      && attr->unreadExpireUnits() != ExpirationCollectionAttribute::ExpireNever
      && daysToExpireUnread >= 0 ) {
    expireUnreadMailCB->setChecked( true );
    expireUnreadMailSB->setValue( daysToExpireUnread );
  }

  if ( attr->expireAction() == ExpirationCollectionAttribute::ExpireDelete )
    deletePermanentlyRB->setChecked( true );
  else
    moveToRB->setChecked( true );

  Akonadi::Collection::Id destFolderID = attr->expireToFolderId();
  if ( destFolderID > 0 ) {
    Akonadi::Collection destFolder = Kernel::self()->collectionFromId( destFolderID );
    if ( destFolder.isValid() )
      folderSelector->setCollection( destFolder );
  }
  slotUpdateControls();
  setAttribute(Qt::WA_WState_Polished);
  if ( mustDeleteExpirationAttribute )
    delete attr;
}

/*
 *  Destroys the object and frees any allocated resources
 */
ExpiryPropertiesDialog::~ExpiryPropertiesDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void ExpiryPropertiesDialog::accept()
{
  bool enableGlobally = expireReadMailCB->isChecked() || expireUnreadMailCB->isChecked();
  const Akonadi::Collection expireToFolder = folderSelector->collection();
  if ( enableGlobally && moveToRB->isChecked() && !expireToFolder.isValid() ) {
    KMessageBox::error( this, i18n("Please select a folder to expire messages into."),
                        i18n( "No Folder Selected" ) );
    return;
  }
  MailCommon::ExpirationCollectionAttribute *attribute = 0;
  if ( expireToFolder.isValid() && moveToRB->isChecked() ) {
    if ( expireToFolder.id() == mFolder.id() ) {
      KMessageBox::error( this, i18n( "Please select a different folder than the current folder to expire message into." ),
                          i18n( "Wrong Folder Selected" ) );
      return;
    }
    else {
      attribute = mFolder.attribute<MailCommon::ExpirationCollectionAttribute>( Akonadi::Entity::AddIfMissing );
      attribute->setExpireToFolderId( expireToFolder.id() );
    }

  }
  if ( !attribute )
    attribute =  mFolder.attribute<MailCommon::ExpirationCollectionAttribute>( Akonadi::Entity::AddIfMissing );

  attribute->setAutoExpire( enableGlobally );
  // we always write out days now
  attribute->setReadExpireAge( expireReadMailSB->value() );
  attribute->setUnreadExpireAge( expireUnreadMailSB->value() );
  attribute->setReadExpireUnits( expireReadMailCB->isChecked() ? MailCommon::ExpirationCollectionAttribute::ExpireDays :
                                 MailCommon::ExpirationCollectionAttribute::ExpireNever );
  attribute->setUnreadExpireUnits( expireUnreadMailCB->isChecked() ? MailCommon::ExpirationCollectionAttribute::ExpireDays :
                                   MailCommon::ExpirationCollectionAttribute::ExpireNever );

  if ( deletePermanentlyRB->isChecked() )
    attribute->setExpireAction( ExpirationCollectionAttribute::ExpireDelete );
  else
    attribute->setExpireAction( ExpirationCollectionAttribute::ExpireMove );
  Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob( mFolder, this );
  job->setProperty( "enableGlobally", enableGlobally );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCollectionModified(KJob*)) );
}

void ExpiryPropertiesDialog::slotCollectionModified(KJob* job)
{
  if ( job->error() ) {
    kDebug()<<" Error when we modified collection";
    return;
  }
  // trigger immediate expiry if there is something to do
  if ( job->property( "enableGlobally" ).toBool() )
    MailCommon::Util::expireOldMessages( mFolder, true /*immediate*/);
  KDialog::accept();

}

void ExpiryPropertiesDialog::slotUpdateControls()
{
  const bool showExpiryActions = expireReadMailCB->isChecked() || expireUnreadMailCB->isChecked();
  moveToRB->setEnabled( showExpiryActions );
  folderSelector->setEnabled( showExpiryActions && moveToRB->isChecked() );
  deletePermanentlyRB->setEnabled( showExpiryActions );

  expireReadMailSB->setEnabled( expireReadMailCB->isChecked() );
  expireUnreadMailSB->setEnabled( expireUnreadMailCB->isChecked() );
}


#include "expirypropertiesdialog.moc"
