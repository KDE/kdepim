/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "storageservicelistwidget.h"
#include <KMenu>

StorageServiceListWidget::StorageServiceListWidget(PimCommon::StorageServiceAbstract::Capabilities capabilities, QWidget *parent)
    : QListWidget(parent),
      mCapabilities(capabilities)
{
    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
}

StorageServiceListWidget::~StorageServiceListWidget()
{

}

void StorageServiceListWidget::slotContextMenu(const QPoint &pos)
{
    const QList<QListWidgetItem *> lstSelectedItems = selectedItems();
    const bool hasItemsSelected = !lstSelectedItems.isEmpty();
    KMenu *menu = new KMenu( this );
    //TODO
    menu->exec( mapToGlobal( pos ) );
    delete menu;
}
