/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <amantia@kdab.com>

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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.BulkActionScreen {
  id : bulkActionScreen
  anchors.fill: parent
  property bool initialized : false
  property alias model: bulkActionHeaderList.model

  actionListWidth : 1/3 * parent.width
  multipleText : KDE.i18np("1 folder", "%1 folders", collectionView.numSelected)
  selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
  headerList : HeaderView {
    id: bulkActionHeaderList
//    model: itemModel
    checkModel : _itemActionModel
    anchors.fill : parent
  }

  QML.Component.onCompleted: {
    if ( initialized == false ) {
      bulkActionScreen.actionModel.set( 0, { "action" : "akonadi_move_to_trash" } )
      bulkActionScreen.actionModel.append({"action": "akonadi_mark_as_read"})
      bulkActionScreen.actionModel.append({"action": "akonadi_mark_as_important"})
      bulkActionScreen.actionModel.append({"action": "akonadi_mark_as_action_item"})
      bulkActionScreen.actionModel.append({"action": "apply_filters_bulk_action"})
      initialized = true;
    }
  }
}
