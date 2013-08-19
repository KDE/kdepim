/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
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

import QtQuick 1.0
import org.kde 4.5
import org.kde.akonadi 4.5 as Akonadi
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi.notes 4.5 as Notes
import "../mobileui/ScreenFunctions.js" as Screen

KPIM.MainView {
  id: notesMobile

  Connections {
    target: guiStateManager
    onGuiStateChanged: { updateContextActionStates() }
  }

  Component.onCompleted : updateContextActionStates();

  function updateContextActionStates()
  {
    if ( guiStateManager.inHomeScreenState ) {
      noteActions.showOnlyCategory( "home" )
    } else if ( guiStateManager.inAccountScreenState ) {
      noteActions.showOnlyCategory( "account" )
    } else if ( guiStateManager.inSingleFolderScreenState ) {
      noteActions.showOnlyCategory( "single_folder" )
    } else if ( guiStateManager.inMultipleFolderScreenState || guiStateManager.inSearchResultScreenState ) {
      noteActions.showOnlyCategory( "multiple_folder" )
    } else if ( guiStateManager.inViewSingleItemState ) {
      noteActions.showOnlyCategory( "note_viewer" )
    }
  }

  NoteView {
    id: noteView
    objectName : "noteView"
    visible: guiStateManager.inViewSingleItemState
    anchors.left: parent.left
    anchors.topMargin : 40
    anchors.bottomMargin : 10
    anchors.leftMargin : 50
    anchors.rightMargin : 10
    width: parent.width
    height: parent.height

    Rectangle {
      anchors.top : noteView.top
      anchors.bottom : noteView.bottom
      anchors.right : noteView.left
      width : noteView.anchors.leftMargin
      color : "#FAFAFA"
    }
  }

  Rectangle {
    id : backToMessageListButton
    visible: guiStateManager.inViewSingleItemState
    anchors.right : notesMobile.right
    anchors.rightMargin : 70
    anchors.bottom : notesMobile.bottom
    anchors.bottomMargin : 100
    Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      MouseArea {
        anchors.fill : parent;
        onClicked : {
          noteView.saveNote();
          _itemNavigationModel.select(-1, 1)
          guiStateManager.popState();
        }
      }
    }
  }


  Item {
    id : mainWorkView
    visible: { guiStateManager.inHomeScreenState ||
               guiStateManager.inAccountScreenState ||
               guiStateManager.inSingleFolderScreenState ||
               guiStateManager.inMultipleFolderScreenState
             }
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right

    Image {
      id: backgroundImage
      x: 0
      y: 0
// FIXME: too big, costs about 1.5Mb RAM
//      source: "notes-mobile-background.png"
      visible: collectionView.visible
    }

    Akonadi.AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      width: 1/3 * parent.width
      anchors.bottom : selectButton.top
      anchors.left: parent.left
      itemHeight: Screen.partition( height, 7 )

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 folders, %2 is e.g. from 2 accounts, %3 is e.g. 9 emails",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 folder","%1 folders",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 note","%1 notes",headerList.count))

      onSelectedClicked : {
        guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState )
      }

      KPIM.AgentStatusIndicator {
        anchors { top: parent.top; right: parent.right; rightMargin: 10; topMargin: 10 }
      }
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
        guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState )
      }
    }

    KPIM.StartCanvas {
      id : startPage
      visible: !collectionView.hasSelection
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      anchors.leftMargin : 10
      anchors.rightMargin : 10
      showAccountsList : false
      favoritesModel : favoritesList

      contextActions : [
        Column {
          anchors.fill: parent
          height : 70
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "New Note" )
            onClicked : {
              application.startComposer();
            }
          }
        }
      ]
    }

    Rectangle {
      id : emptyFolderPage
      visible: (!guiStateManager.inHomeScreenState && collectionView.hasBreadcrumbs && headerList.count == 0)
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      Text {
        text : KDE.i18n("No notes in this notebook");
        height : 20;
        font.italic : true
        horizontalAlignment : Text.AlignHCenter
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
    }

    Rectangle {
      id : notesListPage
      visible: { guiStateManager.inAccountScreenState ||
                 guiStateManager.inSingleFolderScreenState ||
                 guiStateManager.inMultipleFolderScreenState
               }
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"

      NotesListView {
        id: headerList
        model: itemModel
        checkModel : _itemActionModel
        anchors.left : parent.left
        anchors.top : parent.top
        anchors.bottom : filterLineEdit.top
        anchors.right : parent.right
        navigationModel : _itemNavigationModel
        itemHeight: Screen.partition( height, 7 )
      }

      Akonadi.FilterLineEdit {
        id: filterLineEdit
        anchors.left : parent.left
        anchors.bottom : parent.bottom
        anchors.right : parent.right
        visible : false
        height : 0
        y: height == 0 ? parent.height : parent.height - height
      }

      Connections {
        target : _itemNavigationModel
        onCurrentRowChanged : {
          headerList.currentRow = _itemNavigationModel.currentRow
          noteView.currentNoteRow = _itemNavigationModel.currentRow
          guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
          _itemActionModel.select( _itemNavigationModel.currentRow, 3 );
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 100

    visible: !guiStateManager.inBulkActionScreenState &&
             !guiStateManager.inMultipleFolderSelectionScreenState &&
             !guiStateManager.inConfigScreenState &&
             !guiStateManager.inSearchScreenState

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      Component.onCompleted: {
        actionPanel.expanded.connect( noteActions, noteActions.refresh );
      }

      content : [
        NoteActions {
          id : noteActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanel.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "search_note"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.SearchScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "add_as_favorite"
              script : {
                actionPanel.collapse();
                application.saveFavorite();
              }
            },
            KPIM.ScriptAction {
              name : "start_maintenance"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
              }
            }
          ]

          onDoCollapse : actionPanel.collapse();
        }
      ]
    }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inMultipleFolderSelectionScreenState ? KDE.locate( "module", "imports/org/kde/pim/mobileui/MultipleSelectionScreen.qml" ) : ""
    onLoaded: { item.backgroundImage = backgroundImage.source; }
  }

  Loader {
    anchors.fill: parent
    source: guiStateManager.inBulkActionScreenState ? "BulkActionComponent.qml" : ""
    onLoaded: {
      item.backgroundImage = backgroundImage.source
      item.itemHeight = Screen.partition( item.height, 7 )
    }
  }

  KPIM.SearchResultScreen {
    id : searchResultScreen
    anchors.fill: parent

    itemView: NotesListView {
      id: searchNotesListView
      model: itemModel
      checkModel : _itemActionModel
      navigationModel : _itemNavigationModel
      anchors.fill : parent
      itemHeight: Screen.partition( height, 7 )
    }

    resultText: KDE.i18np( "One note found", "%1 notes found", searchNotesListView.count )
  }

  Connections {
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
    }
  }

  Loader {
    id : aboutDialog
    anchors.fill: parent
    visible: false
    source: visible ? KDE.locate( "module", "imports/org/kde/pim/mobileui/AboutDialog.qml" ) : ""
    onLoaded: { item.backgroundSource = backgroundImage.source; }
  }


  KPIM.SearchDialog {
    id : searchDialog
    searchWidget: Notes.SearchWidget {
      anchors.fill: parent
    }
  }
}
