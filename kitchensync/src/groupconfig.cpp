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
#include <libqopensync/result.h>
#include <libqopensync/group.h>
#include <libqopensync/plugin.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <kpagewidget.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QTimer>

GroupConfig::GroupConfig( QWidget *parent )
  : QWidget( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( KDialog::spacingHint() );

  QFrame *titleFrame = new QFrame( this );
  titleFrame->setAutoFillBackground( true );
  topLayout->addWidget( titleFrame );

  QPalette pal;
  pal.setColor( titleFrame->foregroundRole(), palette().color( QPalette::Light ) );
  pal.setColor( titleFrame->backgroundRole(), palette().color( QPalette::Mid ) );
  titleFrame->setPalette( pal );

  QBoxLayout *nameLayout = new QHBoxLayout( titleFrame );
  nameLayout->setMargin( 0 );

  QPixmap icon = KIconLoader::global()->loadIcon( "view-pim-summary", KIconLoader::Desktop );

  QLabel *iconLabel = new QLabel( titleFrame );
  iconLabel->setPixmap( icon );
  nameLayout->addWidget( iconLabel );

  nameLayout->addSpacing( 8 );

  QLabel *label = new QLabel( i18n("Group:"), titleFrame );
  QFont font = label->font();
  font.setBold( true );
  font.setPointSize( font.pointSize() + 2 );
  label->setFont( font );
  nameLayout->addWidget( label );

  mNameLabel = new QLabel( titleFrame );
  font = mNameLabel->font();
  font.setBold( true );
  font.setPointSize( font.pointSize() + 2 );
  mNameLabel->setFont( font );
  nameLayout->addWidget( mNameLabel );

  nameLayout->addStretch( 1 );

  mMemberView = new KPageWidget( this );
  mMemberView->setFaceType( KPageWidget::List );
  topLayout->addWidget( mMemberView );

  QFrame *page = new QFrame();
  KPageWidgetItem *item = mMemberView->addPage( page, i18n("Group") );
  item->setHeader( i18n("General Group Settings") );
  item->setIcon( KIcon( "bookmarks" ) );

  QBoxLayout *pageLayout = new QVBoxLayout( page );

  mCommonConfig = new GroupConfigCommon( page );
  pageLayout->addWidget( mCommonConfig );

  connect( mMemberView, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ),
           SLOT( memberWidgetSelected( KPageWidgetItem*, KPageWidgetItem* ) ) );

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
  QMap<QWidget*, MemberConfig*>::ConstIterator memberIt;
  for ( memberIt = mMemberConfigs.begin(); memberIt != mMemberConfigs.end(); ++memberIt )
    memberIt.value()->saveData();

  QList<KPageWidgetItem*>::ConstIterator it2;
  for ( it2 = mConfigPages.begin(); it2 != mConfigPages.end(); ++it2 ) {
    mMemberView->removePage( *it2 );
  }

  mConfigPages.clear();
  mMemberConfigs.clear();

  const QSync::Group group = mProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    QSync::Member member = group.memberAt( i );
    MemberInfo mi( member );
    QFrame *page = new QFrame();

    KPageWidgetItem *item = mMemberView->addPage( page, mi.name() );
    item->setHeader( QString( "%1 (%2)" ).arg( mi.name() ).arg( member.pluginName() ) );
    item->setIcon( KIcon( mi.iconName() ) );

    QBoxLayout *pageLayout = new QVBoxLayout( page );
    mConfigPages.append( item );

    MemberConfig *memberConfig = new MemberConfig( page, member );
    mMemberConfigs.insert( page, memberConfig );
    pageLayout->addWidget( memberConfig );

    memberConfig->loadData();
  }
}

void GroupConfig::saveConfig()
{
  mProcess->group().save();

  QMap<QWidget*, MemberConfig*>::ConstIterator it;
  for ( it = mMemberConfigs.begin(); it != mMemberConfigs.end(); ++it )
    it.value()->saveData();

  mCommonConfig->save();

  const QSync::Group group = mProcess->group();
  for ( int i = 0; i < group.memberCount(); ++i ) {
    const QSync::Member member = group.memberAt( i );
    mProcess->engine()->discover( member );
  }

  mProcess->reinitEngine();
}

void GroupConfig::memberWidgetSelected( KPageWidgetItem *current, KPageWidgetItem* )
{
  /**
   * Emit 'true' whenever a real member widget is selected by the
   * user.
   */
  emit memberSelected( current->widget() != mCommonConfig->parentWidget() );
}

void GroupConfig::addMember()
{
  QSync::Plugin plugin = PluginPickerDialog::getPlugin( this );

  if ( plugin.isValid() ) {
    QSync::Result result = SyncProcessManager::self()->addMember( mProcess, plugin );
    if ( result.isError() ) {
      KMessageBox::error( this, i18n("Error adding member %1\n%2\nType: %3",
        plugin.name(), result.message(), result.type() ) );
    } else {
      updateMembers();

      // select last (added) page
      mMemberView->setCurrentPage( mConfigPages.last() );
    }
  }
}

void GroupConfig::removeMember()
{
  QWidget *currentPage = mMemberView->currentPage()->widget();
  if ( currentPage && mMemberConfigs.contains( currentPage ) ) {
    MemberConfig *config = mMemberConfigs[ currentPage ];

    SyncProcessManager::self()->removeMember( mProcess, config->member() );
    mMemberConfigs.remove( currentPage );
    QTimer::singleShot( 0, this, SLOT( updateMembers() ) );
  }
}

#include "groupconfig.moc"
