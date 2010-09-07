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

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpixmap.h>
#include <tqprogressbar.h>
#include <tqvbox.h>

#include <libqopensync/plugin.h>
#include <libqopensync/pluginenv.h>

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
  TQFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  TQGridLayout *layout = new TQGridLayout( this, 4, 4, KDialog::marginHint(), KDialog::spacingHint() );

  mBox = new TQVBox( this );
  mBox->setMargin( 5 );
  mProgressBar = new TQProgressBar( this );
  mProgressBar->setTotalSteps( 100 );

  mTime = new TQLabel( this );
  mSyncAction = new KURLLabel( "exec:/sync", i18n( "Synchronize Now" ), this );
  mConfigureAction = new KURLLabel( "exec:/config", i18n( "Configure" ), this );

  // header
  TQHBox* hbox = new TQHBox( this );
  hbox->setMargin( 2 );

  static TQPixmap icon;
  if ( icon.isNull() )
    icon = KGlobal::iconLoader()->loadIcon( "kontact_summary", KIcon::Desktop );

  mIcon = new TQLabel( hbox );
  mIcon->setPixmap( icon );
  mIcon->setFixedSize( mIcon->sizeHint() );
  mIcon->setPaletteBackgroundColor( colorGroup().mid() );

  mGroupName = new TQLabel( hbox );
  mGroupName->setAlignment( AlignLeft | AlignVCenter );
  mGroupName->setIndent( KDialog::spacingHint() );
  mGroupName->setFont( boldFont );
  mGroupName->setPaletteForegroundColor( colorGroup().light() );
  mGroupName->setPaletteBackgroundColor( colorGroup().mid() );

  mStatus = new TQLabel( hbox );
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

  connect( mCallbackHandler, TQT_SIGNAL( conflict( QSync::SyncMapping ) ),
           this, TQT_SLOT( conflict( QSync::SyncMapping ) ) );
  connect( mCallbackHandler, TQT_SIGNAL( change( const QSync::SyncChangeUpdate& ) ),
           this, TQT_SLOT( change( const QSync::SyncChangeUpdate& ) ) );
  connect( mCallbackHandler, TQT_SIGNAL( mapping( const QSync::SyncMappingUpdate& ) ),
           this, TQT_SLOT( mapping( const QSync::SyncMappingUpdate& ) ) );
  connect( mCallbackHandler, TQT_SIGNAL( engine( const QSync::SyncEngineUpdate& ) ),
           this, TQT_SLOT( engine( const QSync::SyncEngineUpdate& ) ) );
  connect( mCallbackHandler, TQT_SIGNAL( member( const QSync::SyncMemberUpdate& ) ),
           this, TQT_SLOT( member( const QSync::SyncMemberUpdate& ) ) );
  connect( mSyncAction, TQT_SIGNAL( leftClickedURL() ),
           this, TQT_SLOT( synchronize() ) );
  connect( mConfigureAction, TQT_SIGNAL( leftClickedURL() ),
           this, TQT_SLOT( configure() ) );
  connect( mSyncProcess, TQT_SIGNAL( engineChanged( QSync::Engine* ) ),
           this, TQT_SLOT( engineChanged( QSync::Engine* ) ) );

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

  TQDateTime dateTime = mSyncProcess->group().lastSynchronization();
  if ( dateTime.isValid() )
    mTime->setText( i18n( "Last synchronized on: %1" ).arg( KGlobal::locale()->formatDateTime( dateTime ) ) );
  else
    mTime->setText( i18n( "Not synchronized yet" ) );

  mProgressBar->reset();
  mProgressBar->hide();

  const QSync::Group group = mSyncProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    MemberItem *item = new MemberItem( mBox, mSyncProcess, group.memberAt( i ) );
    item->show();
    item->setStatusMessage( i18n( "Ready" ) );
    mMemberItems.append( item );
  }
}

void GroupItem::clear()
{
  mGroupName->setText( TQString() );

  TQValueList<MemberItem*>::Iterator it;
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
    case QSync::SyncChangeUpdate::Read:
      mProcessedItems++;
      mStatus->setText( i18n( "%1 entries read" ).arg( mProcessedItems ) );
      break;
    case QSync::SyncChangeUpdate::Written:
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
    case QSync::SyncChangeUpdate::Error:
      mStatus->setText( i18n( "Error" ) );
      KPassivePopup::message( update.result().message(), this );
      break;
    default:
      mStatus->setText( TQString() );
      break;
  }
}

void GroupItem::mapping( const QSync::SyncMappingUpdate& )
{
}

void GroupItem::engine( const QSync::SyncEngineUpdate &update )
{
  switch ( update.type() ) {
    case QSync::SyncEngineUpdate::Connected:
      mStatus->setText( i18n( "Connected" ) );
      mProgressBar->setProgress( 0 );
      mSynchronizing = true;
      mSyncAction->setText( "Abort Synchronization" );
      break;
    case QSync::SyncEngineUpdate::Read:
      mStatus->setText( i18n( "Data read" ) );
      break;
    case QSync::SyncEngineUpdate::Written:
      mStatus->setText( i18n( "Data written" ) );
      mProgressBar->setProgress( 100 );
      mProcessedItems = mMaxProcessedItems = 0;
      break;
    case QSync::SyncEngineUpdate::Disconnected:
      mStatus->setText( i18n( "Disconnected" ) );
      break;
    case QSync::SyncEngineUpdate::Error:
      mStatus->setText( i18n( "Synchronization failed" ) );
      KPassivePopup::message( update.result().message(), this );
      this->update();

      mSynchronizing = false;
      mSyncAction->setText( i18n( "Synchronize Now" ) );
      break;
    case QSync::SyncEngineUpdate::SyncSuccessful:
      mStatus->setText( i18n( "Successfully synchronized" ) );
      mSyncProcess->group().setLastSynchronization( TQDateTime::currentDateTime() );
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
      mStatus->setText( TQString() );
      break;
  }
}

void GroupItem::member( const QSync::SyncMemberUpdate &update )
{
  TQValueList<MemberItem*>::Iterator it;
  for ( it = mMemberItems.begin(); it != mMemberItems.end(); ++it ) {
    if ( (*it)->member() == update.member() ) {
      switch ( update.type() ) {
        case QSync::SyncMemberUpdate::Connected:
          (*it)->setStatusMessage( i18n( "Connected" ) );
          break;
        case QSync::SyncMemberUpdate::Read:
          (*it)->setStatusMessage( i18n( "Changes read" ) );
          break;
        case QSync::SyncMemberUpdate::Written:
          (*it)->setStatusMessage( i18n( "Changes written" ) );
          break;
        case QSync::SyncMemberUpdate::Disconnected:
          (*it)->setStatusMessage( i18n( "Disconnected" ) );
          break;
        case QSync::SyncMemberUpdate::SyncDone:
          (*it)->setStatusMessage( i18n( "Synchronization done" ) );
          break;
        case QSync::SyncMemberUpdate::Discovered:
          (*it)->setStatusMessage( i18n( "Discovered" ) );
          break;
        case QSync::SyncMemberUpdate::Error:
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

MemberItem::MemberItem( TQWidget *parent, SyncProcess *process,
                        const QSync::Member &member )
  : TQWidget( parent ), mSyncProcess( process ), mMember( member )
{
  TQFont boldFont;
  boldFont.setBold( true );

  const MemberInfo mi( member );
  const TQPixmap icon = mi.smallIcon();

  TQVBoxLayout *layout = new TQVBoxLayout( this );

  TQHBox* box = new TQHBox( this );
  box->setMargin( 5 );
  box->setSpacing( 6 );
  layout->addWidget( box );

  mIcon = new TQLabel( box );
  mIcon->setPixmap( icon );
  mIcon->setAlignment( Qt::AlignTop );
  mIcon->setFixedWidth( mIcon->sizeHint().width() );

  TQVBox *nameBox = new TQVBox( box );
  mMemberName = new TQLabel( nameBox );
  mMemberName->setFont( boldFont );
  mDescription = new TQLabel( nameBox );

  mStatus = new TQLabel( box );

  mMemberName->setText( member.name() );

  const QSync::PluginEnv *env = SyncProcessManager::self()->pluginEnv();
  const QSync::Plugin plugin = env->pluginByName( member.pluginName() );

  if ( plugin.isValid() )
    mDescription->setText( plugin.longName() );
  else
    mDescription->setText( i18n("Plugin \"%1\" can't get initialized!").arg( member.pluginName() ) );
}

void MemberItem::setStatusMessage( const TQString &msg )
{
  mStatus->setText( msg );
}

#include "groupitem.moc"
