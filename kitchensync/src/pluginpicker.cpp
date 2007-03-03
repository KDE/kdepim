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

#include <libqopensync/environment.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

PluginItem::PluginItem( KWidgetList *list, const QSync::Plugin &plugin )
  : KWidgetListItem( list ), mPlugin( plugin )
{
  QString iconName = MemberInfo::pluginIconName( mPlugin.name() );
  QGridLayout *layout = new QGridLayout( this, 2, 2, KDialog::marginHint(), KDialog::spacingHint() );

  QLabel *icon = new QLabel( this );
  icon->setPixmap( KGlobal::iconLoader()->loadIcon( iconName, KIcon::Desktop ) );
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

  mPluginList->setFocus();
}

void PluginPicker::updatePluginList()
{
  mPluginList->clear();

  QSync::Environment *env = SyncProcessManager::self()->environment();

  QSync::Environment::PluginIterator it( env->pluginBegin() );
  for( ; it != env->pluginEnd(); ++it ) {
    QSync::Plugin plugin = *it;
    mPluginList->appendItem( new PluginItem( mPluginList, plugin ) );
  }
}

QSync::Plugin PluginPicker::selectedPlugin() const
{
  PluginItem *item = static_cast<PluginItem *>( mPluginList->selectedItem() );
  if ( item ) return item->plugin();
  else return QSync::Plugin();
}


PluginPickerDialog::PluginPickerDialog( QWidget *parent )
  : KDialogBase( parent, 0, true, i18n("Select Member Type"), Ok | Cancel )
{
  QFrame *topFrame = makeMainWidget();

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mPicker = new PluginPicker( topFrame );
  topLayout->addWidget( mPicker );

  connect( mPicker, SIGNAL( selected() ), SLOT( slotOk() ) );

  setInitialSize( QSize( 460, 380 ) );
}

QSync::Plugin PluginPickerDialog::selectedPlugin() const
{
  return mPicker->selectedPlugin();
}

QSync::Plugin PluginPickerDialog::getPlugin( QWidget *parent )
{
  PluginPickerDialog dlg( parent );
  if ( dlg.exec() )
    return dlg.selectedPlugin();
  else
    return QSync::Plugin();
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
