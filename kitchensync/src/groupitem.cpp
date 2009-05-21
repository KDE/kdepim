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
#include <kcolorscheme.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knotification.h>
#include <kurllabel.h>

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QProgressBar>
#include <QtGui/QVBoxLayout>

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
  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 1 );
  layout->setSpacing( 3 );

  mBox = new QWidget( this );
  mBoxLayout = new QVBoxLayout( mBox );
  mBoxLayout->setMargin( 1 );

  mProgressBar = new QProgressBar( this );
  mProgressBar->setRange( 0, 100 );

  mTime = new QLabel( this );
  mSyncAction = new KUrlLabel( "exec:/sync", i18n( "Synchronize Now" ), this );
  mConfigureAction = new KUrlLabel( "exec:/config", i18n( "Configure" ), this );

  // header
  QWidget *hbox = new QWidget( this );
  QHBoxLayout *hboxLayout = new QHBoxLayout( hbox );
  hboxLayout->setMargin( 2 );

  static QPixmap icon;
  if ( icon.isNull() ) {
    icon = KIconLoader::global()->loadIcon( "bookmarks",
                                            KIconLoader::Desktop );
  }

  mIcon = new QLabel( hbox );
  mIcon->setPixmap( icon );
  mIcon->setFixedSize( mIcon->sizeHint() );
  hboxLayout->addWidget( mIcon );

  QPalette pal1;
  pal1.setColor( mIcon->backgroundRole(), palette().color( QPalette::Mid ) );
  mIcon->setPalette( pal1 );

  mGroupName = new QLabel( hbox );
  mGroupName->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  mGroupName->setIndent( KDialog::spacingHint() );
  mGroupName->setFont( boldFont );
  mGroupName->setAutoFillBackground( true );
  hboxLayout->addWidget( mGroupName );

  QPalette pal2;
  pal2.setColor( mGroupName->foregroundRole(), palette().color( QPalette::Light ) );
  pal2.setColor( mGroupName->backgroundRole(), palette().color( QPalette::Mid ) );
  mGroupName->setPalette( pal2 );

  mStatus = new QLabel( hbox );
  mStatus->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  mStatus->setIndent( KDialog::spacingHint() );
  mStatus->setFont( boldFont );
  mStatus->setText( i18n( "Ready" ) );
  mStatus->setAutoFillBackground( true );
  hboxLayout->addWidget( mStatus );

  QPalette pal3;
  pal3.setColor( mStatus->foregroundRole(), palette().color( QPalette::Light ) );
  pal3.setColor( mStatus->backgroundRole(), palette().color( QPalette::Mid ) );
  mStatus->setPalette( pal3 );

  QPalette pal4;
  pal4.setColor( hbox->backgroundRole(), palette().color( QPalette::Mid ) );
  hbox->setAutoFillBackground( true );
  hbox->setPalette( pal4 );

  hbox->setMaximumHeight( hbox->minimumSizeHint().height() );

  layout->addWidget( hbox, 0, 0, 1, 4 );
  layout->addWidget( mBox, 1, 0, 1, 4 );
  layout->addWidget( mTime, 2, 0 );
  layout->addWidget( mSyncAction, 2, 1 );
  layout->addWidget( mConfigureAction, 2, 2 );
  layout->addWidget( mProgressBar, 2, 3 );
  layout->setColumnStretch( 0, 1 );
  layout->setRowStretch( 3, 1 );

  QPalette pal5;
  pal5.setColor( backgroundRole(), kapp->palette().color( QPalette::Active, QPalette::Base ) );
  setPalette( pal5 );
  setAutoFillBackground( true );

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
  connect( mSyncAction, SIGNAL( leftClickedUrl() ),
           this, SLOT( synchronize() ) );
  connect( mConfigureAction, SIGNAL( leftClickedUrl() ),
           this, SLOT( configure() ) );
  connect( mSyncProcess, SIGNAL( engineChanged( QSync::Engine* ) ),
           this, SLOT( engineChanged( QSync::Engine* ) ) );

  mCallbackHandler->setEngine( mSyncProcess->engine() );

  setSelectionForegroundColor( KColorScheme( QPalette::Active, KColorScheme::View ).foreground().color() );
  setSelectionBackgroundColor( KColorScheme( QPalette::Active, KColorScheme::View ).background( KColorScheme::AlternateBackground ).color() );

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

  mGroupName->setText( i18n( "Group: %1", mSyncProcess->group().name() ) );

  QDateTime dateTime = mSyncProcess->group().lastSynchronization();
  if ( dateTime.isValid() ) {
    mTime->setText( i18n( "Last synchronized on: %1",
                          KGlobal::locale()->formatDateTime( dateTime ) ) );
  } else {
    mTime->setText( i18n( "Not synchronized yet" ) );
  }

  mProgressBar->reset();
  mProgressBar->hide();

  const QSync::Group group = mSyncProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    MemberItem *item = new MemberItem( mBox, mSyncProcess, group.memberAt( i ) );
    mBoxLayout->addWidget( item );
    item->show();
    item->setStatusMessage( i18n( "Ready" ) );
    mMemberItems.append( item );
  }
}

void GroupItem::clear()
{
  mGroupName->setText( QString() );

  for ( int i = 0; i < mMemberItems.count(); ++i ) {
    delete mMemberItems[ i ];
  }

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
      mStatus->setText( i18np( "1 entry read", "%1 entries read", mProcessedItems ) );
      break;
    case QSync::SyncChangeUpdate::Written:
      mProcessedItems--;
      mStatus->setText( i18np( "1 entry written", "%1 entries written", mMaxProcessedItems - mProcessedItems ) );

      mProgressBar->show();

      {
        int progress = 100;
        if ( mMaxProcessedItems != 0 ) {
          progress = (mProcessedItems * 100) / mMaxProcessedItems;
        }

        if ( progress < 0 ) {
          progress = 0;
        }

        mProgressBar->setValue( 100 - progress );
      }
      break;
    case QSync::SyncChangeUpdate::Error:
      mStatus->setText( i18n( "Error" ) );
      KNotification::event(KNotification::Error, update.result().message(), QPixmap(), this );
      break;
    default:
      mStatus->setText( QString() );
      break;
  }
}

void GroupItem::mapping( const QSync::SyncMappingUpdate &update )
{
  Q_UNUSED( update );
}

void GroupItem::engine( const QSync::SyncEngineUpdate &update )
{
  switch ( update.type() ) {
    case QSync::SyncEngineUpdate::Connected:
      mStatus->setText( i18n( "Connected" ) );
      mProgressBar->setValue( 0 );
      mSynchronizing = true;
      mSyncAction->setText( i18n("Abort Synchronization") );
      break;
    case QSync::SyncEngineUpdate::Read:
      mStatus->setText( i18n( "Data read" ) );
      break;
    case QSync::SyncEngineUpdate::Written:
      mStatus->setText( i18n( "Data written" ) );
      mProgressBar->setValue( 100 );
      mProcessedItems = mMaxProcessedItems = 0;
      break;
    case QSync::SyncEngineUpdate::Disconnected:
      mStatus->setText( i18n( "Disconnected" ) );
      break;
    case QSync::SyncEngineUpdate::Error:
      mStatus->setText( i18n( "Synchronization failed" ) );
      KNotification::event(KNotification::Error, update.result().message(), QPixmap(), this );
      this->update();

      mSynchronizing = false;
      mSyncAction->setText( i18n( "Synchronize Now" ) );
      break;
    case QSync::SyncEngineUpdate::Successful:
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
  for ( int i = 0; i < mMemberItems.count(); ++i ) {
    MemberItem *item = mMemberItems[ i ];

    if ( item->member() == update.member() ) {
      switch ( update.type() ) {
        case QSync::SyncMemberUpdate::Connected:
          item->setStatusMessage( i18n( "Connected" ) );
          break;
        case QSync::SyncMemberUpdate::Read:
          item->setStatusMessage( i18n( "Changes read" ) );
          break;
        case QSync::SyncMemberUpdate::Written:
          item->setStatusMessage( i18n( "Changes written" ) );
          break;
        case QSync::SyncMemberUpdate::Disconnected:
          item->setStatusMessage( i18n( "Disconnected" ) );
          break;
        case QSync::SyncMemberUpdate::SyncDone:
          item->setStatusMessage( i18n( "Synchronization done" ) );
          break;
        case QSync::SyncMemberUpdate::Discovered:
          item->setStatusMessage( i18n( "Discovered" ) );
          break;
        case QSync::SyncMemberUpdate::Error:
          item->setStatusMessage( i18n( "Error: %1", update.result().message() ) );
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
  if ( !mSynchronizing ) {
    emit synchronizeGroup( mSyncProcess );
  } else {
    emit abortSynchronizeGroup( mSyncProcess );
  }
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

  const QSync::PluginEnv *env = SyncProcessManager::self()->pluginEnv();
  const QSync::Plugin plugin = env->pluginByName( member.pluginName() );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 1 );

  QWidget *box = new QWidget( this );
  QHBoxLayout *boxLayout = new QHBoxLayout( box );
  boxLayout->setMargin( 3 );
  boxLayout->setSpacing( 3 );
  layout->addWidget( box );

  mIcon = new QLabel( box );
  mIcon->setPixmap( icon );
  mIcon->setAlignment( Qt::AlignTop );
  mIcon->setFixedWidth( mIcon->sizeHint().width() );
  boxLayout->addWidget( mIcon );

  QWidget *nameBox = new QWidget( box );
  QVBoxLayout *nameBoxLayout = new QVBoxLayout( nameBox );
  boxLayout->addWidget( nameBox );

  mMemberName = new QLabel( nameBox );
  mMemberName->setFont( boldFont );
  nameBoxLayout->addWidget( mMemberName );

  mDescription = new QLabel( nameBox );
  nameBoxLayout->addWidget( mDescription );

  mStatus = new QLabel( box );
  boxLayout->addWidget( mStatus );

  mMemberName->setText( member.name() );

  if ( plugin.isValid() )
    mDescription->setText( plugin.longName() );
}

void MemberItem::setStatusMessage( const QString &msg )
{
  mStatus->setText( msg );
}

#include "groupitem.moc"
