/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

import Qt 4.6
import org.kde 4.5
import org.kde.akonadi 4.5
import org.kde.messageviewer 4.5
import "HeaderView.qml"

Rectangle {
  id: topLevel
  color: "white"
  height: 480
  width: 800

  SystemPalette { id: palette; colorGroup: "Active" }
  Rectangle {
    id: navbar
    width: parent.width
    height: 56
    anchors.top: parent.top
    color: palette.window
    Image {
      id: messageViewIcon
      height: 48
      width: 48
      anchors.right: parent.right
      source: KDE.iconPath( "internet-mail", height )
      MouseArea { anchors.fill: parent; onClicked: topLevel.state = "messageViewState" }
    }
    Image {
      height: 48
      width: height
      anchors.right: messageViewIcon.left
      source: KDE.iconPath( "folder", height )
      MouseArea {
        anchors.fill: parent
        onClicked: topLevel.state = "collectionListState"
      }
    }
  }

  Rectangle {
    id: homeScreen
    width: parent.width
    height: parent.height - navbar.height
    anchors.top: navbar.bottom
    color: "blue"
    Row {
      anchors.fill: parent
      Image {
        id: icon
        height: Math.min( parent.height, 256 )
        width: height
        anchors.verticalCenter: parent.verticalCenter
        source: KDE.iconPath( "kmail", height )
      }
      Text {
        text: "Welcome to KMail Mobile!!!"
        anchors.verticalCenter: icon.verticalCenter
      }
    }
    MouseArea  {
      anchors.fill: parent
      onClicked: topLevel.state = "collectionListState"
    }
  }

  CollectionView {
    id: collectionList
    width: parent.width
    height: parent.height - navbar.height
    anchors.top: navbar.bottom
    model: collectionModel
    onCollectionSelected: topLevel.state = "headerListState"
  }

  HeaderView {
    id: headerList
    width: parent.width
    height: parent.height - navbar.height
    anchors.top: navbar.bottom
    model: itemModel
    onMessageSelected: topLevel.state = "messageViewState"
  }

  Component {
    id : messageViewDelegate

    MessageView {
      id: messageView
      width: messageViewList.width
      height: messageViewList.height
      anchors.top: navbar.bottom
      messageItemId: model.itemId
      splashMessage: KDE.i18n( "Welcome to KMail Mobile." )
    }
  }

  ListView {
    id: messageViewList
    model : itemModel
    delegate : del1

    anchors.top: navbar.bottom
    anchors.left : parent.left
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem
    flickDeceleration: 2000
  }

  states: [
    State {
      name: "homeScreenState"
      PropertyChanges {
        target: homeScreen
        visible: true
      }
      PropertyChanges {
        target: collectionList
        visible: false
      }
      PropertyChanges {
        target: headerList
        visible: false
      }
      PropertyChanges {
        target: messageViewList
        visible: false
      }
    },
    State {
      name: "collectionListState"
      PropertyChanges {
        target: homeScreen
        visible: false
      }
      PropertyChanges {
        target: collectionList
        visible: true
      }
      PropertyChanges {
        target: headerList
        visible: false
      }
      PropertyChanges {
        target: messageViewList
        visible: false
      }
    },
    State {
      name: "headerListState"
      PropertyChanges {
        target: homeScreen
        visible: false
      }
      PropertyChanges {
        target: collectionList
        visible: false
      }
      PropertyChanges {
        target: headerList
        visible: true
      }
      PropertyChanges {
        target: messageViewList
        visible: false
      }
    },
    State {
      name: "messageViewState"
      PropertyChanges {
        target: homeScreen
        visible: false
      }
      PropertyChanges {
        target: collectionList
        visible: false
      }
      PropertyChanges {
        target: headerList
        visible: false
      }
      PropertyChanges {
        target: messageViewList
        visible: true
      }
    }
  ]

  state: "homeScreenState"

  Binding {
    target: application
    property: "collectionRow"
    value: collectionList.currentIndex
  }

}
