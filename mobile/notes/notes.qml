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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.akonadi 4.5 as Akonadi
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.MainView {
  id: notesMobile

  QML.SystemPalette { id: palette; colorGroup: "Active" }

  NoteView {
    id: noteView
    anchors.left: parent.left
    anchors.topMargin : 40
    anchors.bottomMargin : 10
    anchors.leftMargin : 50
    anchors.rightMargin : 10
    width: parent.width
    height: parent.height
    visible : false
  }

  QML.Rectangle {
    id : backToMessageListButton
    anchors.right : notesMobile.right
    anchors.rightMargin : 70
    anchors.bottom : notesMobile.bottom
    anchors.bottomMargin : 100
    visible : false
    QML.Image {
      source : "back-to-message-list.png"
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          noteView.saveNote();
          noteView.visible = false;
          backToMessageListButton.visible = false;
          collectionView.visible = true;
          notesListPage.visible = true;
          noteView.noteId = -1;
        }
      }
    }
  }


  QML.Item {
    id : mainWorkView
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right

    Akonadi.AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      width: 1/3 * parent.width
      anchors.bottom : selectButton.top
      //height : parent.height - ( collectionView.hasSelection ? 0 : selectButton.height)
      anchors.left: parent.left

      multipleSelectionText : KDE.i18na("You have selected \n%1 folders\nfrom %2 accounts\n%3 notes", [collectionView.numSelected,
                                                                                                        application.numSelectedAccounts,
                                                                                                        headerList.count])
      breadcrumbItemsModel : breadcrumbCollectionsModel
      selectedItemModel : selectedCollectionModel
      childItemsModel : childCollectionsModel
    }
    KPIM.Button2 {
      id : selectButton
      anchors.left: collectionView.left
      anchors.right: collectionView.right
      anchors.bottom : parent.bottom
      anchors.bottomMargin : { (collectionView.numSelected == 1) ? -selectButton.height : 0 }
      buttonText : (collectionView.numSelected <= 1) ? KDE.i18n("Select") : KDE.i18n("Change Selection")
      opacity : { (collectionView.numSelected == 1) ? 0 : 1 }
      onClicked : {
        application.persistCurrentSelection("preFavSelection");
        favoriteSelector.visible = true;
        mainWorkView.visible = false;
      }
    }

    KPIM.StartCanvas {
      id : startPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      anchors.leftMargin : 10
      anchors.rightMargin : 10

      opacity : collectionView.hasSelection ? 0 : 1
      showAccountsList : false
      favoritesModel : favoritesList

      contextActions : [
        QML.Column {
          anchors.fill: parent
          height : 70
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "Write new Note" )
            onClicked : {
//               application.startComposer();
            }
          }
        }
      ]
    }

    QML.Rectangle {
      id : emptyFolderPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : (collectionView.hasBreadcrumbs && headerList.count == 0 ) ? 1 : 0
      QML.Text {
        text : KDE.i18n("No notes in this notebook");
        height : 20;
        font.italic : true
        horizontalAlignment : QML.Text.AlignHCenter
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
    }

    QML.Rectangle {
      id : notesListPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : headerList.count > 0 ? 1 : 0

      NotesListView {
        id: headerList
        model: itemModel
        anchors.fill : parent
        onItemSelected: {
          // Prevent reloading of the message, perhaps this should be done
          // in messageview itself.
          if ( noteView.noteId != headerList.currentItemId )
          {
            noteView.noteId = headerList.currentItemId;
            noteView.currentNoteRow = -1;
            noteView.currentNoteRow = headerList.currentIndex;

            noteView.visible = true;
            backToMessageListButton.visible = true;
            collectionView.visible = false;
            notesListPage.visible = false;
          }
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

/*
    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "view-pim-notes", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: KDE.i18n( "Notes start page" )

          contextActions : [
            KPIM.Button {
              id : start_newNoteButton
              height : 20
              width : 200
              anchors.top : parent.top
              buttonText : KDE.i18n( "New Note" )
              onClicked : {
                console.log( "New Note clicked" );
              }

            },
            KPIM.Button {
              id : start_newNotebookButton
              anchors.top : start_newNoteButton.bottom
              height : 20
              width : 200
              buttonText : KDE.i18n( "Add Notebook" )
              onClicked : {
                console.log( "Add Notebook clicked" );
//                 application.launchAccountWizard();
              }
            }
          ]
        }
      ]
    }
    SlideoutPanel {
      id: folderPanel
      titleText: KDE.i18n( "Notebooks" )
      handleHeight: 150
      anchors.fill : parent
      content: [
        QML.Item {
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
                 height : itemListView.height / 7
                 summaryContent: [
                  QML.Column {
                    anchors.fill: parent
                    QML.Text {
                      text: KDE.i18na( "Title: %1", [model.title] )
                    }
                    QML.Text {
                      text: KDE.i18na( "Content: %1", [model.plainContent] )
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
*/
    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      anchors.fill : parent
      contentWidth: 240
//       content: [
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
//       ]
    }
  }

  QML.Connections {
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
    }
  }

   QML.Connections {
     target: collectionView
     onChildCollectionSelected : { application.setSelectedChildCollectionRow( row ); }
   }

   QML.Connections {
     target: collectionView
     onBreadcrumbCollectionSelected : { application.setSelectedBreadcrumbCollectionRow( row ); }
   }
}
