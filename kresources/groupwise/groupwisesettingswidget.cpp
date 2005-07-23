/*
    This file is part of kdepim.

    Copyright (c) 2005 Will Stephenson <wstephenson@suse.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <klistview.h>

#include "groupwisesettingswidget.h"

GroupWiseSettingsWidget::GroupWiseSettingsWidget( QWidget * parent )
: GroupWiseSettingsWidgetBase( parent )
{
  connect( m_settingsList, SIGNAL( itemRenamed( QListViewItem *, int ) ),
          this, SLOT( slotItemRenamed( QListViewItem *, int ) ) );
}

void GroupWiseSettingsWidget::slotItemRenamed( QListViewItem * item, int )
{
  kdDebug() << "GroupWiseSettingsWidget::slotItemRenamed() - " << item->text( 1 ) << " changed to " << item->text( 2 ) << endl;
  m_dirtySettings.insert( item->text( 1 ), item->text( 2 ) );
}

QMap<QString, QString> GroupWiseSettingsWidget::dirtySettings()
{
  return m_dirtySettings;
}

void GroupWiseSettingsWidget::reset()
{
  m_dirtySettings.clear();
}

#include "groupwisesettingswidget.moc"
