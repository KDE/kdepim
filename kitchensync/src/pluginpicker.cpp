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

#include <tqlabel.h>
#include <tqlayout.h>

PluginItem::PluginItem( KWidgetList *list, const QSync::Plugin &plugin )
  : KWidgetListItem( list ), mPlugin( plugin )
{
  TQString iconName = MemberInfo::pluginIconName( mPlugin.name() );
  TQGridLayout *layout = new TQGridLayout( this, 2, 2, KDialog::marginHint(), KDialog::spacingHint() );

  TQLabel *icon = new TQLabel( this );
  icon->setPixmap( KGlobal::iconLoader()->loadIcon( iconName, KIcon::Desktop ) );
  icon->setFixedSize( icon->sizeHint() );

  TQLabel *name = new TQLabel( plugin.longName(), this );
  TQLabel *description = new TQLabel( plugin.description(), this );

  TQFont font = name->font();
  font.setBold( true );
  name->setFont( font );

  layout->addWidget( icon, 0, 0 );
  layout->addWidget( name, 0, 1 );
  layout->addWidget( description, 1, 1 );
}


PluginPicker::PluginPicker( TQWidget *parent )
  : TQWidget( parent )
{
  TQBoxLayout *layout = new TQVBoxLayout( this );

  mPluginList = new KWidgetList( this );
  layout->addWidget( mPluginList );

  connect( mPluginList, TQT_SIGNAL( doubleClicked( KWidgetListItem* ) ),
           TQT_SIGNAL( selected() ) );

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


PluginPickerDialog::PluginPickerDialog( TQWidget *parent )
  : KDialogBase( parent, 0, true, i18n("Select Member Type"), Ok | Cancel )
{
  TQFrame *topFrame = makeMainWidget();

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mPicker = new PluginPicker( topFrame );
  topLayout->addWidget( mPicker );

  connect( mPicker, TQT_SIGNAL( selected() ), TQT_SLOT( slotOk() ) );

  setInitialSize( TQSize( 460, 380 ) );
}

QSync::Plugin PluginPickerDialog::selectedPlugin() const
{
  return mPicker->selectedPlugin();
}

QSync::Plugin PluginPickerDialog::getPlugin( TQWidget *parent )
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
