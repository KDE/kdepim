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
import org.kde.akonadi 4.5 as Akonadi
import org.kde.pim.mobileui 4.5 as KPIM

QML.Rectangle {
  color : "white"
  id : _top
  property alias selectedItemModel : actionList.selectedItemModel
  property alias multipleText : actionList.multipleText
  property alias headerList : headerList.children
  property alias actionListWidth : actionList.width
  property alias backgroundImage : _backgroundImage.source
  property alias actionModel : actionList.actionModel
  property alias itemHeight : actionList.itemHeight
  default property alias actions: actionList.children

  anchors.fill: parent
  anchors.topMargin : 12

  QML.Image {
    id: _backgroundImage
  }

  BulkActionList {
    id : actionList
    anchors.top : parent.top
    anchors.bottom : selectAllButton.top
    anchors.left : parent.left
    onBackClicked : {
      guiStateManager.popState()
      bulkActionFilterLineEdit.clear()
    }
    onTriggered : {
      //mainPanel.complete(name)
    }
  }

  KPIM.Button2 {
    id: selectAllButton
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.right: mainPanel.left
    buttonText: stateHolder.state == "check_all" ? KDE.i18n( "Select All" ) : KDE.i18n( "Deselect All" )
    onClicked: {
      if ( stateHolder.state == "check_all" ) {
        application.checkAllBulkActionItems( true )
        stateHolder.state = "uncheck_all"
      } else {
        application.checkAllBulkActionItems( false )
        stateHolder.state = "check_all"
      }
    }

    QML.Item {
      id: stateHolder
      state: "check_all"

      states: [
        QML.State {
          name: "check_all"
        },

        QML.State {
          name: "uncheck_all"
        }
      ]
    }
  }

  QML.Item {
    id : mainPanel
    anchors.top : parent.top
    anchors.right : parent.right
    anchors.bottom : parent.bottom
    anchors.left : actionList.right

    QML.Item {
      anchors.left : parent.left
      anchors.top : parent.top
      anchors.bottom : bulkActionFilterLineEdit.top
      anchors.right : parent.right
      id : headerList
    }

    Akonadi.BulkActionFilterLineEdit {
      id: bulkActionFilterLineEdit
      anchors.left : parent.left
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      visible : false
      height : 0
      y: height == 0 ? parent.height : parent.height - height
    }
  }
}
