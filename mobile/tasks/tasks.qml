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
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.MainView {
  id: tasksMobile

  SystemPalette { id: palette; colorGroup: "Active" }

  Rectangle { // TaskView
    id: taskView
    anchors.left: parent.left
    width: parent.width
    height: parent.height

//     z: 0
//
//     messageItemId: -1
//     swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev
//
//     onNextMessageRequest: {
//       // Only go to the next message when currently a valid item is set.
//       if ( messageView.messageItemId >= 0 )
//         headerList.nextMessage();
//     }
//
//     onPreviousMessageRequest: {
//       // Only go to the previous message when currently a valid item is set.
//       if ( messageView.messageItemId >= 0 )
//         headerList.previousMessage();
//     }
  }


  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      id: folderPanel
      titleText: "Folders"
      handleHeight: 150
      anchors.fill : parent
      content: [
        Item {
          anchors.fill: parent

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
               //folderPanel.collapse()
             }
           }

           KPIM.ItemListView {
             id: itemList
             delegate: [
               KPIM.ItemListViewDelegate {
                 summaryContent: [
                   Text {
                     anchors.fill: parent
                     text: "Task: " + model.summary
                     font.bold: true
                   }
                 ]
                 detailsContent: [
                    Column {
                      anchors.fill: parent
                      Text {
                        text: "Task: " + model.summary
                        color: palette.highlightedText
                        font.bold: true
                      }
                      Text {
                        text: "Details: " + model.description
                        color: palette.highlightedText
                      }
                    }
                 ]
               }
             ]

             model: itemModel
             anchors.top: parent.top
             anchors.bottom: parent.bottom
             anchors.right: parent.right
             anchors.left: collectionView.right
             onItemSelected: {
               // Prevent reloading of the message, perhaps this should be done
               // in messageview itself.
//               if ( messageView.messageItemId != headerList.currentMessage )
//                 messageView.messageItemId = headerList.currentMessage;
               folderPanel.collapse()
             }
           }
        }
      ]
    }

    SlideoutPanel {
      id: actionPanel
      titleText: "Actions"
      handleHeight: 150
      handlePosition : 150
      anchors.fill : parent
      contentWidth: 240
      content: [
          Text {
            id: actionLabel
            text: "Actions"
            style: Text.Sunken
            anchors.horizontalCenter: parent.horizontalCenter
          },
          KPIM.Button {
            id: moveButton
            anchors.top: actionLabel.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: "Move"
            onClicked: actionPanel.collapse();
          },
          KPIM.Button {
             id: deleteButton
             anchors.top: moveButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Delete"
             onClicked: actionPanel.collapse();
           },
           KPIM.Button {
             id: previousButton
             anchors.top: deleteButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Previous"
             onClicked: {
               itemList.previousItem()
               actionPanel.collapse()
             }
           },
           KPIM.Button {
             anchors.top: previousButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: "Next"
             onClicked: {
               itemList.nextItem();
               actionPanel.collapse();
             }
           }
      ]
    }

    SlideoutPanel {
      id: attachmentPanel

      handlePosition : 300
      anchors.fill : parent
      visible: false //messageView.messageTreeModel.attachmentCount >= 2
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handlePosition: folderPanel.handleHeight + actionPanel.handleHeight
      handleHeight: parent.height - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      contentWidth: 400
//      content: [
//         Component {
//           id: attachmentDelegate
//           Item {
//             id: wrapper
//             width: attachmentList.width
//             height: 48
//             clip: true
//             Rectangle {
//               anchors.fill: parent
//               opacity: 0.25
//               border.color: palette.mid
//             }
//             Text { anchors.fill: parent; text: model.display; horizontalAlignment: "AlignHCenter"; verticalAlignment: "AlignVCenter"; color: "black" }
//           }
//         },
//         ListView {
//           id: attachmentList
//           anchors.fill: parent
//
//           model: messageView.messageTreeModel
//           delegate: attachmentDelegate
//
//           MouseArea {
//             anchors.fill: parent
//             onClicked: {
//               console.log( "current mime tree count: " + messageView.messageTreeModel.attachmentCount );
//             }
//           }
//         }
//      ]
    }
  }

   Connections {
     target: collectionView
     onChildCollectionSelected : { application.setSelectedChildCollectionRow( row ); }
   }

   Connections {
     target: collectionView
     onBreadcrumbCollectionSelected : { application.setSelectedBreadcrumbCollectionRow( row ); }
   }
}
