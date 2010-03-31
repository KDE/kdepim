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
    anchors.topMargin: 20
    anchors.rightMargin: 20
    anchors.bottomMargin: 10
    titleText: "Folders"
    handleHeight: 150
    content: [
      Item {
        anchors.fill: parent
        anchors.margins: 12

      /*CollectionView {
        id: collectionList
        anchors.fill: parent
        anchors.margins: 12
        model: collectionModel
        onCollectionSelected: {
          folderPanel.collapse()
          messageViewListFixingTimerHack.start()
        }*/
        BreadcrumbNavigationView {
          id : collectionView
          width: 1/3 * folderPanel.contentWidth
          anchors.top: parent.top
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.rightMargin: 4
          breadcrumbItemsModel : breadcrumbCollectionsModel
          selectedItemModel : selectedCollectionModel
          childItemsModel : childCollectionsModel
          onCollectionSelected: {
            console.log( "XXX" );
            //folderPanel.collapse()
            messageViewListFixingTimerHack.start()
          }
        }

        HeaderView {
          id: headerList
          model: itemModel
          anchors.top: parent.top
          anchors.bottom: parent.bottom
          anchors.right: parent.right
          anchors.left: collectionView.right
          onMessageSelected: {
            console.log( "YYY" );
            var existingSpeed = messageViewList.highlightMoveSpeed;
            messageViewList.highlightMoveSpeed = -1;
            messageViewList.currentIndex = headerList.currentIndex
            messageViewList.highlightMoveSpeed = existingSpeed;
            folderPanel.collapse()
          }
        }
      }
    ]
  }

  SlideoutPanel {
    id: actionPanel
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.rightMargin: 20
    anchors.bottomMargin: 10
    titleText: "Actions"
    handlePosition: folderPanel.handleHeight
    handleHeight: 150
    contentWidth: 240
    content: [
        Text {
          text: "Actions"
          style: Text.Sunken
          anchors.horizontalCenter: parent.horizontalCenter
        },  
        Button {
          id: moveButton
          anchors.top: parent.top;
          anchors.margins: 12
          width: parent.width
          height: parent.height / 6
          buttonText: "Move"
        },
        Button {
          id: deleteButton
          anchors.top: moveButton.bottom;
          anchors.margins: 12
          width: parent.width
          height: parent.height / 6
          buttonText: "Delete"
        },
        Button {
          id: previousButton
          anchors.top: deleteButton.bottom;
          anchors.margins: 12
          width: parent.width
          height: parent.height / 6
          buttonText: "Previous"
        },
        Button {
          anchors.top: previousButton.bottom;
          anchors.margins: 12
          width: parent.width
          height: parent.height / 6
          buttonText: "Next"
        }
    ]
  }

  SlideoutPanel {
    id: attachmentPanel
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.rightMargin: 20
    anchors.bottomMargin: 10
    titleIcon: KDE.iconPath( "mail-attachment", 48 );
    handlePosition: folderPanel.handleHeight + actionPanel.handleHeight
    handleHeight: parent.height - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
    contentWidth: 400
    content: [
      Component {
        id: attachmentDelegate
        Item {
          id: wrapper
          width: attachmentList.width
          height: 48
          clip: true
          Rectangle {
            anchors.fill: parent
            opacity: 0.25
            border.color: palette.mid
          }
          Text { anchors.fill: parent; text: model.display; horizontalAlignment: "AlignHCenter"; verticalAlignment: "AlignVCenter"; color: "black" }
        }
      },
      ListView {
        id: attachmentList
        anchors.fill: parent
        anchors.margins: 12
        model: messageViewList.currentItem.messageTreeModel
        delegate: attachmentDelegate

        MouseArea {
          anchors.fill: parent
          onClicked: {
            console.log( "current index: " + messageViewList.currentIndex );
            console.log( "current item: " + messageViewList.currentItem );
            console.log( "model: " + messageViewList.model );
            console.log( "model count: " + messageViewList.model.count );
            console.log( "current mime tree count: " + messageViewList.currentItem.messageTreeModel.count );
          }
        }
      }
    ]
  }

  /** Placeholder to prevent mailviewers show up under the tabs */
  Rectangle {
    x: 0
    width: folderPanel.handleWidth
    height: parent.height
  }

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
    width: parent.width - folderPanel.handleWidth
    height: parent.height
    model: itemModel
    delegate: messageViewDelegate
    preferredHighlightBegin: 0
    preferredHighlightEnd: 0
    highlightRangeMode: "StrictlyEnforceRange"
    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem
    flickDeceleration: 200
  }

  Button {
    id: floatingDeleteButton
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: 20
    z: 5

    width: 80
    height: 80
    opacity: 0.5
    visible: messageViewList.count > 0
    icon: KDE.iconPath( "user-trash", 64 )
    onClicked: console.log( "please, delete current akonadi item" );
  }

  Connections {
    target: collectionView
    onChildCollectionSelected : { application.selectedChildCollectionRow(row); }
  }

  Timer {
    id: messageViewListFixingTimerHack
    interval: 500
    repeat: false
    running: false
    onTriggered: {
      messageViewList.currentIndex = 0
    }
  }

  Connections {
    target: collectionView
    onBreadcrumbCollectionSelected : { application.selectedBreadcrumbCollectionRow(row); }
  }
}
