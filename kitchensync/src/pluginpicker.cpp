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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "pluginpicker.h"

#include "memberinfo.h"
#include "syncprocessmanager.h"

#include <libqopensync/pluginenv.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include <QtGui/QLabel>
#include <QtGui/QLayout>

PluginItem::PluginItem( KWidgetList *list, const QSync::Plugin &plugin )
  : KWidgetListItem( list ), mPlugin( plugin )
{
  QString iconName = MemberInfo::pluginIconName( mPlugin.name() );
  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );

  QLabel *icon = new QLabel( this );
  icon->setPixmap( KIconLoader::global()->loadIcon( iconName, K3Icon::Desktop ) );
  icon->setFixedSize( icon->sizeHint() );

  QLabel *name = new QLabel( plugin.longName(), this );
  QLabel *description = new QLabel( plugin.description(), this );

  QFont font = name->font();
  font.setBold( true );
  name->setFont( font );

  layout->addWidget( icon, 0, 0 );
  layout->addWidget( name, 0, 1 );
  layout->addWidget( description, 1, 1 );
}

PluginPicker::PluginPicker( QWidget *parent )
  : QWidget( parent )
{
  QBoxLayout *layout = new QVBoxLayout( this );

  mPluginList = new KWidgetList( this );
  layout->addWidget( mPluginList );

  connect( mPluginList, SIGNAL( doubleClicked( KWidgetListItem* ) ),
           SIGNAL( selected() ) );

  updatePluginList();

  setFocusProxy( mPluginList );
}

void PluginPicker::updatePluginList()
{
  mPluginList->clear();

  QSync::PluginEnv *env = SyncProcessManager::self()->pluginEnv();

  for ( int i = 0; i < env->pluginCount(); ++i ) {
    QSync::Plugin plugin = env->pluginAt( i );
    mPluginList->appendItem( new PluginItem( mPluginList, plugin ) );
  }
}

QSync::Plugin PluginPicker::selectedPlugin() const
{
  PluginItem *item = static_cast<PluginItem *>( mPluginList->selectedItem() );
  if ( item ) {
    return item->plugin();
  } else {
    return QSync::Plugin();
  }
}

PluginPickerDialog::PluginPickerDialog( QWidget *parent )
  : KDialog( parent )
{
  setModal( true );
  setWindowTitle( i18n("Select Member Type") );
  setButtons( Ok | Cancel );

  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mPicker = new PluginPicker( topFrame );
  topLayout->addWidget( mPicker );

  connect( mPicker, SIGNAL( selected() ), SLOT( slotOk() ) );

  setInitialSize( QSize( 460, 380 ) );

  mPicker->setFocus();

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( this, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
}

QSync::Plugin PluginPickerDialog::selectedPlugin() const
{
  return mPicker->selectedPlugin();
}

QSync::Plugin PluginPickerDialog::getPlugin( QWidget *parent )
{
  PluginPickerDialog dlg( parent );
  if ( dlg.exec() ) {
    return dlg.selectedPlugin();
  } else {
    return QSync::Plugin();
  }
}

void PluginPickerDialog::slotOk()
{
  accept();
}

void PluginPickerDialog::slotCancel()
{
  reject();
}

#include "pluginpicker.moc"
