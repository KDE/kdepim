/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kurllabel.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qvbox.h>

#include "memberinfo.h"
#include "multiconflictdialog.h"
#include "singleconflictdialog.h"
#include "syncprocessmanager.h"

#include "groupitem.h"

GroupItem::GroupItem( KWidgetList *parent, SyncProcess *process )
  : KWidgetListItem( parent ), mSyncProcess( process ),
    mCallbackHandler( new QSync::CallbackHandler ),
    mProcessedItems( 0 ), mMaxProcessedItems( 0 ),
    mSynchronizing( false )
{
  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QGridLayout *layout = new QGridLayout( this, 4, 4, KDialog::marginHint(), KDialog::spacingHint() );

  mBox = new QVBox( this );
  mBox->setMargin( 5 );
  mProgressBar = new QProgressBar( this );
  mProgressBar->setTotalSteps( 100 );

  mTime = new QLabel( this );
  mSyncAction = new KURLLabel( "exec:/sync", i18n( "Synchronize Now" ), this );
  mConfigureAction = new KURLLabel( "exec:/config", i18n( "Configure" ), this );

  // header
  QHBox* hbox = new QHBox( this );
  hbox->setMargin( 2 );

  static QPixmap icon;
  if ( icon.isNull() )
    icon = KGlobal::iconLoader()->loadIcon( "kontact_summary", KIcon::Desktop );

  mIcon = new QLabel( hbox );
  mIcon->setPixmap( icon );
  mIcon->setFixedSize( mIcon->sizeHint() );
  mIcon->setPaletteBackgroundColor( colorGroup().mid() );

  mGroupName = new QLabel( hbox );
  mGroupName->setAlignment( AlignLeft | AlignVCenter );
  mGroupName->setIndent( KDialog::spacingHint() );
  mGroupName->setFont( boldFont );
  mGroupName->setPaletteForegroundColor( colorGroup().light() );
  mGroupName->setPaletteBackgroundColor( colorGroup().mid() );

  mStatus = new QLabel( hbox );
  mStatus->setAlignment( Qt::AlignRight );
  mStatus->setAlignment( AlignRight | AlignVCenter );
  mStatus->setIndent( KDialog::spacingHint() );
  mStatus->setFont( boldFont );
  mStatus->setPaletteForegroundColor( colorGroup().light() );
  mStatus->setPaletteBackgroundColor( colorGroup().mid() );
  mStatus->setText( i18n( "Ready" ) );

  hbox->setPaletteBackgroundColor( colorGroup().mid() );
  hbox->setMaximumHeight( hbox->minimumSizeHint().height() );

  layout->addMultiCellWidget( hbox, 0, 0, 0, 3 );
  layout->addMultiCellWidget( mBox, 1, 1, 0, 3 );
  layout->addWidget( mTime, 2, 0 );
  layout->addWidget( mSyncAction, 2, 1 );
  layout->addWidget( mConfigureAction, 2, 2 );
  layout->addWidget( mProgressBar, 2, 3 );
  layout->setColStretch( 0, 1 );
  layout->setRowStretch( 3, 1 );

  setPaletteBackgroundColor( kapp->palette().active().base() );

  connect( mCallbackHandler, SIGNAL( conflict( QSync::SyncMapping ) ),
           this, SLOT( conflict( QSync::SyncMapping ) ) );
  connect( mCallbackHandler, SIGNAL( change( const QSync::SyncChangeUpdate& ) ),
           this, SLOT( change( const QSync::SyncChangeUpdate& ) ) );
  connect( mCallbackHandler, SIGNAL( mapping( const QSync::SyncMappingUpdate& ) ),
           this, SLOT( mapping( const QSync::SyncMappingUpdate& ) ) );
  connect( mCallbackHandler, SIGNAL( engine( const QSync::SyncEngineUpdate& ) ),
           this, SLOT( engine( const QSync::SyncEngineUpdate& ) ) );
  connect( mCallbackHandler, SIGNAL( member( const QSync::SyncMemberUpdate& ) ),
           this, SLOT( member( const QSync::SyncMemberUpdate& ) ) );
  connect( mSyncAction, SIGNAL( leftClickedURL() ),
           this, SLOT( synchronize() ) );
  connect( mConfigureAction, SIGNAL( leftClickedURL() ),
           this, SLOT( configure() ) );
  connect( mSyncProcess, SIGNAL( engineChanged( QSync::Engine* ) ),
           this, SLOT( engineChanged( QSync::Engine* ) ) );

  mCallbackHandler->setEngine( mSyncProcess->engine() );

  setSelectionForegroundColor( KGlobalSettings::textColor() );
  setSelectionBackgroundColor( KGlobalSettings::alternateBackgroundColor() );

  update();
}

GroupItem::~GroupItem()
{
  delete mCallbackHandler;
  mCallbackHandler = 0;
}

void GroupItem::update()
{
  clear();

  mGroupName->setText( i18n( "Group: %1" ).arg( mSyncProcess->group().name() ) );

  QDateTime dateTime = mSyncProcess->group().lastSynchronization();
  if ( dateTime.isValid() )
    mTime->setText( i18n( "Last synchronized on: %1" ).arg( KGlobal::locale()->formatDateTime( dateTime ) ) );
  else
    mTime->setText( i18n( "Not synchronized yet" ) );

  mProgressBar->reset();
  mProgressBar->hide();

  QSync::Group group = mSyncProcess->group();
  QSync::Group::Iterator memberIt( group.begin() );
  QSync::Group::Iterator memberEndIt( group.end() );

  for ( ; memberIt != memberEndIt; ++memberIt ) {
    MemberItem *item = new MemberItem( mBox, mSyncProcess, *memberIt );
    item->show();
    item->setStatusMessage( i18n( "Ready" ) );
    mMemberItems.append( item );
  }
}

void GroupItem::clear()
{
  mGroupName->setText( QString() );

  QValueList<MemberItem*>::Iterator it;
  for ( it = mMemberItems.begin(); it != mMemberItems.end(); ++it )
    delete *it;

  mMemberItems.clear();
}

void GroupItem::conflict( QSync::SyncMapping mapping )
{
  if ( mapping.changesCount() == 2 ) {
    SingleConflictDialog dlg( mapping, this );
    dlg.exec();
  } else {
    MultiConflictDialog dlg( mapping, this );
    dlg.exec();
  }
}

void GroupItem::change( const QSync::SyncChangeUpdate &update )
{
  switch ( update.type() ) {
    case QSync::SyncChangeUpdate::Received:
      mProcessedItems++;
      mStatus->setText( i18n( "%1 entries read" ).arg( mProcessedItems ) );
      break;
    case QSync::SyncChangeUpdate::ReceivedInfo:
      mStatus->setText( i18n( "Receive information" ) );
      break;
    case QSync::SyncChangeUpdate::Sent:
      mProcessedItems--;
      mStatus->setText( i18n( "%1 entries written" ).arg( mMaxProcessedItems - mProcessedItems ) );

      mProgressBar->show();

      {
        int progress = 100;
        if ( mMaxProcessedItems != 0 )
          progress = (mProcessedItems * 100) / mMaxProcessedItems;

        if ( progress < 0 )
          progress = 0;

        mProgressBar->setProgress( 100 - progress );
      }
      break;
    case QSync::SyncChangeUpdate::WriteError:
      mStatus->setText( i18n( "Error" ) );
      KPassivePopup::message( update.result().message(), this );
      break;
    case QSync::SyncChangeUpdate::ReceiveError:
      mStatus->setText( i18n( "Error" ) );
      KPassivePopup::message( update.result().message(), this );
      break;
    default:
      mStatus->setText( QString() );
      break;
  }
}

void GroupItem::mapping( const QSync::SyncMappingUpdate& )
{
}

void GroupItem::engine( const QSync::SyncEngineUpdate &update )
{
  switch ( update.type() ) {
    case QSync::SyncEngineUpdate::EndPhaseConnected:
      mStatus->setText( i18n( "Connected" ) );
      mProgressBar->setProgress( 0 );
      mSynchronizing = true;
      mSyncAction->setText( "Abort Synchronization" );
      break;
    case QSync::SyncEngineUpdate::EndPhaseRead:
      mStatus->setText( i18n( "Data read" ) );
      break;
    case QSync::SyncEngineUpdate::EndPhaseWrite:
      mStatus->setText( i18n( "Data written" ) );
      mProgressBar->setProgress( 100 );
      mProcessedItems = mMaxProcessedItems = 0;
      break;
    case QSync::SyncEngineUpdate::EndPhaseDisconnected:
      mStatus->setText( i18n( "Disconnected" ) );
      break;
    case QSync::SyncEngineUpdate::Error:
      mStatus->setText( i18n( "Synchronization failed" ) );
      KPassivePopup::message( update.result().message(), this );
      this->update();

      mSynchronizing = false;
      mSyncAction->setText( i18n( "Synchronize Now" ) );
      break;
    case QSync::SyncEngineUpdate::SyncSuccessfull:
      mStatus->setText( i18n( "Successfully synchronized" ) );
      mSyncProcess->group().setLastSynchronization( QDateTime::currentDateTime() );
      mSyncProcess->group().save();
      this->update();

      mSynchronizing = false;
      mSyncAction->setText( i18n( "Synchronize Now" ) );
      break;
    case QSync::SyncEngineUpdate::PrevUnclean:
      mStatus->setText( i18n( "Previous synchronization failed" ) );
      break;
    case QSync::SyncEngineUpdate::EndConflicts:
      mStatus->setText( i18n( "Conflicts solved" ) );
      mMaxProcessedItems = mProcessedItems;
      break;
    default:
      mStatus->setText( QString() );
      break;
  }
}

void GroupItem::member( const QSync::SyncMemberUpdate &update )
{
  QValueList<MemberItem*>::Iterator it;
  for ( it = mMemberItems.begin(); it != mMemberItems.end(); ++it ) {
    if ( (*it)->member() == update.member() ) {
      switch ( update.type() ) {
        case QSync::SyncMemberUpdate::Connected:
          (*it)->setStatusMessage( i18n( "Connected" ) );
          break;
        case QSync::SyncMemberUpdate::SentChanges:
          (*it)->setStatusMessage( i18n( "Changes read" ) );
          break;
        case QSync::SyncMemberUpdate::CommittedAll:
          (*it)->setStatusMessage( i18n( "Changes written" ) );
          break;
        case QSync::SyncMemberUpdate::Disconnected:
          (*it)->setStatusMessage( i18n( "Disconnected" ) );
          break;
        case QSync::SyncMemberUpdate::ConnectError:
          (*it)->setStatusMessage( i18n( "Error: %1" ).arg( update.result().message() ) );
          break;
        case QSync::SyncMemberUpdate::GetChangesError:
          (*it)->setStatusMessage( i18n( "Error: %1" ).arg( update.result().message() ) );
          break;
        case QSync::SyncMemberUpdate::CommittedAllError:
          (*it)->setStatusMessage( i18n( "Error: %1" ).arg( update.result().message() ) );
          break;
        case QSync::SyncMemberUpdate::SyncDoneError:
          (*it)->setStatusMessage( i18n( "Error: %1" ).arg( update.result().message() ) );
          break;
        case QSync::SyncMemberUpdate::DisconnectedError:
          (*it)->setStatusMessage( i18n( "Error: %1" ).arg( update.result().message() ) );
          break;
        default:
          break;
      }

      return;
    }
  }
}

void GroupItem::synchronize()
{
  if ( !mSynchronizing )
    emit synchronizeGroup( mSyncProcess );
  else
    emit abortSynchronizeGroup( mSyncProcess );
}

void GroupItem::configure()
{
  emit configureGroup( mSyncProcess );

  this->update();
}

void GroupItem::engineChanged( QSync::Engine *engine )
{
  Q_ASSERT( engine );

  mCallbackHandler->setEngine( engine );

  this->update();
}

MemberItem::MemberItem( QWidget *parent, SyncProcess *process,
                        const QSync::Member &member )
  : QWidget( parent ), mSyncProcess( process ), mMember( member )
{
  QFont boldFont;
  boldFont.setBold( true );

  MemberInfo mi( member );

  QPixmap icon = mi.smallIcon();

  QSync::Plugin plugin = member.plugin();

  QVBoxLayout *layout = new QVBoxLayout( this );

  QHBox* box = new QHBox( this );
  box->setMargin( 5 );
  box->setSpacing( 6 );
  layout->addWidget( box );

  mIcon = new QLabel( box );
  mIcon->setPixmap( icon );
  mIcon->setAlignment( Qt::AlignTop );
  mIcon->setFixedWidth( mIcon->sizeHint().width() );

  QVBox *nameBox = new QVBox( box );
  mMemberName = new QLabel( nameBox );
  mMemberName->setFont( boldFont );
  mDescription = new QLabel( nameBox );

  mStatus = new QLabel( box );

  mMemberName->setText( member.name() );
  mDescription->setText( plugin.longName() );
}

void MemberItem::setStatusMessage( const QString &msg )
{
  mStatus->setText( msg );
}

#include "groupitem.moc"
