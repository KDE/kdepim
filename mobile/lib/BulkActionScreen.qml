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

import Qt 4.7 as QML
import org.kde.akonadi 4.5 as Akonadi

QML.Rectangle {
  color : "white"
  id : _top
  property alias selectedItemModel : actionList.selectedItemModel
  property alias multipleText : actionList.multipleText
  property alias headerList : headerList.children
  property alias actionListWidth : actionList.width
  property alias backgroundImage : _backgroundImage.source
  property alias actionModel : actionList.actionModel
  default property alias actions: actionList.children

  signal backClicked()

  QML.Image {
    id: _backgroundImage
  }

  BulkActionList {
    id : actionList
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : parent.left
    onBackClicked : parent.backClicked()
    onTriggered : {
      //mainPanel.complete(name)
    }
  }

  QML.Item {
    id : mainPanel
    anchors.top : parent.top
    anchors.right : parent.right
    anchors.bottom : parent.bottom
    anchors.left : actionList.right

    Akonadi.BulkActionFilterLineEdit {
      id: bulkActionFilterLineEdit
      anchors.left : parent.left
      anchors.top : parent.top
      anchors.bottom : headerList.top
      anchors.right : parent.right
      visible : false
      height : 0
    }

    QML.Item {
      anchors.left : parent.left
      anchors.top : bulkActionFilterLineEdit.bottom
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      id : headerList
    }
  }
}
