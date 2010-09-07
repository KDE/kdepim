/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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

#include "groupconfig.h"

#include "groupconfigcommon.h"
#include "memberconfig.h"
#include "memberinfo.h"
#include "pluginpicker.h"
#include "syncprocess.h"
#include "syncprocessmanager.h"

#include <libqopensync/engine.h>
#include <libqopensync/group.h>
#include <libqopensync/plugin.h>
#include <libqopensync/result.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <klocale.h>
#include <kmessagebox.h>


#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtimer.h>

GroupConfig::GroupConfig( TQWidget *parent )
  : TQWidget( parent )
{
  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  TQFrame *titleFrame = new TQFrame( this );
  topLayout->addWidget( titleFrame );

  titleFrame->setPaletteForegroundColor( colorGroup().light() );
  titleFrame->setPaletteBackgroundColor( colorGroup().mid() );

  TQBoxLayout *nameLayout = new TQHBoxLayout( titleFrame );
  nameLayout->setMargin( 4 );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_summary",
    KIcon::Desktop );

  TQLabel *iconLabel = new TQLabel( titleFrame );
  iconLabel->setPixmap( icon );
  nameLayout->addWidget( iconLabel );

  nameLayout->addSpacing( 8 );

  TQLabel *label = new TQLabel( i18n("Group:"), titleFrame );
  TQFont font = label->font();
  font.setBold( true );
  font.setPointSize( font.pointSize() + 2 );
  label->setFont( font );
  nameLayout->addWidget( label );

  mNameLabel = new TQLabel( titleFrame );
  font = mNameLabel->font();
  font.setBold( true );
  font.setPointSize( font.pointSize() + 2 );
  mNameLabel->setFont( font );
  nameLayout->addWidget( mNameLabel );

  nameLayout->addStretch( 1 );

  mMemberView = new KJanusWidget( this, 0, KJanusWidget::IconList );
  topLayout->addWidget( mMemberView );

  icon = KGlobal::iconLoader()->loadIcon( "bookmark", KIcon::Desktop );
  TQFrame *page = mMemberView->addPage( i18n("Group"),
    i18n("General Group Settings"), icon );
  TQBoxLayout *pageLayout = new TQVBoxLayout( page );

  mCommonConfig = new GroupConfigCommon( page );
  pageLayout->addWidget( mCommonConfig );

  connect( mMemberView, TQT_SIGNAL( aboutToShowPage( TQWidget* ) ), TQT_SLOT( memberWidgetSelected( TQWidget* ) ) );
}

void GroupConfig::setSyncProcess( SyncProcess *process )
{
  mProcess = process;

  mNameLabel->setText( mProcess->group().name() );
  mCommonConfig->setSyncProcess( mProcess );

  updateMembers();
}

void GroupConfig::updateMembers()
{
  TQMap<TQWidget*, MemberConfig *>::ConstIterator memberIt;
  for ( memberIt = mMemberConfigs.begin(); memberIt != mMemberConfigs.end(); ++memberIt )
    memberIt.data()->saveData();

  TQValueList<TQFrame *>::ConstIterator it2;
  for ( it2 = mConfigPages.begin(); it2 != mConfigPages.end(); ++it2 ) {
    mMemberView->removePage( *it2 );
    delete *it2;
  }
  mConfigPages.clear();
  mMemberConfigs.clear();

  const QSync::Group group = mProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    QSync::Member member = group.memberAt( i );
    MemberInfo mi( member );
    TQFrame *page = mMemberView->addPage( mi.name(), 
      TQString( "%1 (%2)" ).arg( mi.name() ).arg(member.pluginName()), mi.desktopIcon() );

    TQBoxLayout *pageLayout = new TQVBoxLayout( page );
    mConfigPages.append( page );

    MemberConfig *memberConfig = new MemberConfig( page, member );
    mMemberConfigs.insert( page, memberConfig );
    pageLayout->addWidget( memberConfig );

    memberConfig->loadData();
  }
}

void GroupConfig::saveConfig()
{
  mProcess->group().save();

  TQMap<TQWidget*, MemberConfig*>::ConstIterator it;
  for ( it = mMemberConfigs.begin(); it != mMemberConfigs.end(); ++it )
    it.data()->saveData();

  mCommonConfig->save();

  const QSync::Group group = mProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    const QSync::Member member = group.memberAt( i );
    mProcess->engine()->discover( member );
  }

  mProcess->reinitEngine();
}

void GroupConfig::memberWidgetSelected( TQWidget *wdg )
{
  /**
   * Emit 'true' whenever a real member widget is selected by the
   * user.
   */
  emit memberSelected( wdg != mCommonConfig->parentWidget() );
}

void GroupConfig::addMember()
{
  QSync::Plugin plugin = PluginPickerDialog::getPlugin( this );

  if ( plugin.isValid() ) {
    QSync::Result result = SyncProcessManager::self()->addMember( mProcess, plugin );
    if ( result.isError() ) {
      KMessageBox::error( this, i18n("Error adding member %1\n%2\nType: %3")
        .arg( plugin.name() ).arg( result.message() ).arg( result.type() ) );
    } else {
      updateMembers();

      // select last (added) page
      int index = mMemberView->pageIndex( mConfigPages.last() );
      mMemberView->showPage( index );
    }
  }
}

void GroupConfig::removeMember()
{
  TQWidget *selectedWidget = mMemberView->pageWidget( mMemberView->activePageIndex() );
  if ( selectedWidget && mMemberConfigs.contains( selectedWidget ) ) {
    MemberConfig *config = mMemberConfigs[ selectedWidget ];

    SyncProcessManager::self()->removeMember( mProcess, config->member() );
    mMemberConfigs.remove( selectedWidget );

    TQTimer::singleShot( 0, this, TQT_SLOT( updateMembers() ) );
  }
}

#include "groupconfig.moc"
