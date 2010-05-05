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
import org.kde.kcal 4.5 as KCal

KPIM.MainView {
  id: tasksMobile

  SystemPalette { id: palette; colorGroup: "Active" }

  KCal.IncidenceView {
    id: taskView
    anchors { fill: parent; topMargin: 48; leftMargin: 48 }
    width: parent.width
    height: parent.height

    z: 0

    itemId: -1
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

    onNextItemRequest: {
      // Only go to the next message when currently a valid item is set.
      if ( taskView.itemId >= 0 )
        itemList.nextItem();
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( taskView.itemId >= 0 )
        itemList.previousItem();
    }
  }


  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "korganizer", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: "Tasks start page"

          contextActions : [
            KPIM.Button {
              id : start_newEmailButton
              width: parent.width
              height: 480 / 6
              buttonText : "Start new task"
              onClicked : {
                console.log( "Write new task clicked" );
              }
            }

//            },
//            KPIM.Button {
//              id : start_newAccountButton
//              anchors.top : start_newEmailButton.bottom
//              height : 20
//              width : 200
//              buttonText : "Add Account"
//              onClicked : {
//                console.log( "Add Account clicked" );
//                application.launchAccountWizard();
//              }
//            }
          ]
        }
      ]
    }

    SlideoutPanel {
      id: folderPanel
      titleText: "Folders"
      handlePosition : 108
      handleHeight: 150
      anchors.fill : parent
      content: [
        Item {
          anchors.fill: parent

           AkonadiBreadcrumbNavigationView {
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
                     anchors.top: parent.top
                     anchors.left: parent.left
                     text: "Task: " + model.summary
                     font.bold: true
                   },
                   Text {
                     anchors.top: parent.top
                     anchors.right: parent.right
                     text: model.percentComplete + "%"
                   }
                 ]
                 detailsContent: [
                    Column {
                      anchors.fill: parent
                      Item {
                        width: parent.width
                        height: summaryLabel.height
                        Text {
                          id: summaryLabel
                          anchors.top: parent.top
                          anchors.left: parent.left
                          text: "Task: " + model.summary
                          font.bold: true
                          color: palette.highlightedText
                        }
                        Text {
                          anchors.top: parent.top
                          anchors.right: parent.right
                          text: model.percentComplete + "%"
                          color: palette.highlightedText
                        }
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
               taskView.itemId = itemList.currentItemId;
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
      anchors.fill : parent
      contentWidth: 240
      content: [
          KPIM.Button {
            id: moveButton
            anchors.top: parent.top
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

      anchors.fill : parent
      visible: false //messageView.messageTreeModel.attachmentCount >= 2
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
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
