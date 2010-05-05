/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>
    Copyright (c) 2010 Stephen Kelly <stephen@kdab.com>

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
  id: notesMobile

  SystemPalette { id: palette; colorGroup: "Active" }

  gradient: Gradient {
    GradientStop { position: 0.0; color: "lightgrey" }
    GradientStop { position: 0.5; color: "grey" }
  }
  NoteView {
    id: noteView
    anchors.left: parent.left
    anchors.topMargin : 40
    anchors.bottomMargin : 10
    anchors.leftMargin : 50
    anchors.rightMargin : 10
    width: parent.width
    height: parent.height
  }


  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "kmail", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: "Notes start page"

          contextActions : [
            KPIM.Button {
              id : start_newNoteButton
              height : 20
              width : 200
              anchors.top : parent.top
              buttonText : "New Note"
              onClicked : {
                console.log( "New Note clicked" );
              }

            },
            KPIM.Button {
              id : start_newNotebookButton
              anchors.top : start_newNoteButton.bottom
              height : 20
              width : 200
              buttonText : "Add Notebook"
              onClicked : {
                console.log( "Add Notebook clicked" );
                application.launchAccountWizard();
              }
            }
          ]
        }
      ]
    }

    SlideoutPanel {
      id: folderPanel
      titleText: "Folders"
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
           }

           KPIM.ItemListView {
             id: headerList
             delegate: [
               KPIM.ItemListViewDelegate {
                 summaryContent: [
                   Text {
                     anchors.fill: parent
                     text: "Title: " + model.title
                     font.bold: true
                   }
                 ]
                 detailsContent: [
                  Column {
                    anchors.fill: parent
                    Text {
                      text: "Title: " + model.title
                      color: palette.highlightedText
                    }
                    Text {
                      text: "Content: " + model.plainContent
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
               if ( noteView.noteId != headerList.currentItemId )
               {
                 noteView.noteId = headerList.currentItemId;
                 noteView.currentNoteRow = -1;
                 noteView.currentNoteRow = headerList.currentIndex;
               }
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
//           Button {
//             id: moveButton
//             anchors.top: actionLabel.bottom;
//             anchors.horizontalCenter: parent.horizontalCenter;
//             width: parent.width - 10
//             height: parent.height / 6
//             buttonText: "Move"
//             onClicked: actionPanel.collapse();
//           },
//           Button {
//             id: deleteButton
//             anchors.top: moveButton.bottom;
//             anchors.horizontalCenter: parent.horizontalCenter;
//             width: parent.width - 10
//             height: parent.height / 6
//             buttonText: "Delete"
//             onClicked: actionPanel.collapse();
//           },
//           Button {
//             id: previousButton
//             anchors.top: deleteButton.bottom;
//             anchors.horizontalCenter: parent.horizontalCenter;
//             width: parent.width - 10
//             height: parent.height / 6
//             buttonText: "Previous"
//             onClicked: {
// //               if ( messageView.messageItemId >= 0 )
// //                 headerList.previousMessage();
//
//               actionPanel.collapse();
//             }
//           },
//           Button {
//             anchors.top: previousButton.bottom;
//             anchors.horizontalCenter: parent.horizontalCenter;
//             width: parent.width - 10
//             height: parent.height / 6
//             buttonText: "Next"
//             onClicked: {
// //               if ( messageView.messageItemId >= 0 )
// //                 headerList.nextMessage();
//
//               actionPanel.collapse();
//             }
//           }
      ]
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
