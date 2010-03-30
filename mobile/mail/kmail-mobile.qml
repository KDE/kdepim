/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

import Qt 4.7
import org.kde 4.5
import org.kde.akonadi 4.5
import org.kde.messageviewer 4.5

Rectangle {
  id: kmailMobile
  height: 480
  width: 800
  
  SystemPalette { id: palette; colorGroup: "Active" }

  SlideoutPanel {
    id: folderPanel
    anchors.fill: parent
    titleText: "Folders"
    handlePosition: 40
    content: [
      CollectionView {
        id: collectionList
        anchors.margins: 12
        anchors.fill: parent
        model: collectionModel
        onCollectionSelected: folderPanel.state = ""
      }
    ]

    states: State {
      name: ""; when: collectionList.onCollectionSelected
    }
  }

//   SlideoutPanel {
//     anchors.fill:parent
//     title: "Actions"
//     handlePosition: 200
//     contentWidth: 200
//     content: [
//       Rectangle {
//         color: "red"
//         anchors.margins: 12
//         anchors.fill: parent
//       }
//     ]
//   }

  Component {
    id : messageViewDelegate

    MessageView {
      id: messageView
      width: messageViewList.width
      height: messageViewList.height
      messageItemId: model.itemId
    }
  }

  ListView {
    id: messageViewList
    x: folderPanel.handleWidth
    y: 0
    width: parent.width - folderPanel.handleWidth
    height: parent.height
    model: itemModel
    delegate: messageViewDelegate
    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem;
    flickDeceleration: 2000
  }

  Binding {
    target: application
    property: "collectionRow"
    value: collectionList.currentIndex
  }
}