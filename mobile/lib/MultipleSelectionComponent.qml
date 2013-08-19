/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi 4.5
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Rectangle {
  color : "#00000000"
  AkonadiBreadcrumbNavigationView {
    id : navigationView

    indentation : 0

    clickToBulkAction : false

    showCheckboxes : true
    checkable : true

    breadcrumbComponentFactory : _multiSelectionComponentFactory

    anchors.top : parent.top
    anchors.left : parent.left
    anchors.bottom : parent.bottom
    width : parent.width / 3
  }

  PlasmaExtras.ScrollArea {
    anchors.left : navigationView.right
    anchors.right : parent.right
    anchors.top : parent.top
    anchors.bottom : parent.bottom

    flickableItem: QML.ListView {
      id : selectedView

      model : _multiSelectionComponentFactory.qmlCheckedItemsModel()

      delegate : CollectionDelegate {
        id : selectedDelegate
        uncheckable : true
        alternatingRowColors : true
        checkModel : _multiSelectionComponentFactory.qmlCheckedItemsCheckModel();
        height : 70
      }
    }
  }
}
