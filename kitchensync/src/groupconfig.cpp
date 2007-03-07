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

#include <libqopensync/group.h>
#include <libqopensync/plugin.h>

#include <kdialog.h>
#include <kiconloader.h>
#include <kpagewidget.h>
#include <klocale.h>
#include <kmessagebox.h>


#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

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

  QPixmap icon = KIconLoader::global()->loadIcon( "kontact_summary", K3Icon::Desktop );

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

  QBoxLayout *buttonLayout = new QHBoxLayout();
  topLayout->addLayout( buttonLayout );

  QPushButton *addButton = new QPushButton( i18n("Add Member..."), this );
  connect( addButton, SIGNAL( clicked() ), SLOT( addMember() ) );
  buttonLayout->addWidget( addButton );

  buttonLayout->addStretch( 1 );

  QFrame *page = new QFrame();
  KPageWidgetItem *item = mMemberView->addPage( page, i18n("Group") );
  item->setHeader( i18n("General Group Settings") );
  item->setIcon( KIcon( "bookmark" ) );

  QBoxLayout *pageLayout = new QVBoxLayout( page );

  mCommonConfig = new GroupConfigCommon( page );
  pageLayout->addWidget( mCommonConfig );
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
  for ( int i = 0; i < mMemberConfigs.count(); ++i )
    mMemberConfigs[ i ]->saveData();

  for ( int i = 0; i < mConfigPages.count(); ++i ) {
    mMemberView->removePage( mConfigPages[ i ] );
  }

  mConfigPages.clear();
  mMemberConfigs.clear();

  QSync::Group group = mProcess->group();
  QSync::Group::Iterator it( group.begin() );
  for ( ; it != group.end(); ++it ) {
    QSync::Member member = *it;
    MemberInfo mi( member );
    QFrame *page = new QFrame();

    KPageWidgetItem *item = mMemberView->addPage( page, mi.name() );
    item->setHeader( QString( "%1 (%2)" ).arg( mi.name() ).arg( member.pluginName() ) );
    item->setIcon( KIcon( mi.iconName() ) );

    QBoxLayout *pageLayout = new QVBoxLayout( page );
    mConfigPages.append( item );

    MemberConfig *memberConfig = new MemberConfig( page, member );
    mMemberConfigs.append( memberConfig );
    pageLayout->addWidget( memberConfig );

    memberConfig->loadData();
  }
}

void GroupConfig::saveConfig()
{
  mProcess->group().save();

  for ( int i = 0; i < mMemberConfigs.count(); ++i )
    mMemberConfigs[ i ]->saveData();

  mCommonConfig->save();

  mProcess->reinitEngine();
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
      mMemberView->setCurrentPage( mConfigPages.last() );
    }
  }
}

#include "groupconfig.moc"
