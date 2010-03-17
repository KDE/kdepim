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
import "HeaderView.qml"

Rectangle {
  id: topLevel
  color: "white"
  height: 480
  width: 800

  Rectangle {
    id: homeScreen
    anchors.fill: parent
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
      onClicked: topLevel.state = 'collectionListState'
    }
  }

  CollectionView {
    id: collectionList
    anchors.fill: parent
    model: collectionModel
    onCollectionSelected: topLevel.state = 'headerListState'
  }

  HeaderView {
    id: headerList
    anchors.fill: parent
    model: itemModel
  }

  states: [
    State {
      name: "homeScreenState"
      PropertyChanges {
        target: homeScreen;
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
    }
  ]

  state: "homeScreenState";

  Binding {
    target: application
    property: "collectionRow"
    value: collectionList.currentIndex
  }

}
