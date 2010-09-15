/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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
import org.kde.kcal 4.5 as KCal

KPIM.MainView {
  id: tasksMobile

  function goBackToListing()
  {
    taskView.visible = false;
    collectionView.visible = true;
    itemListPage.visible = true;
    selectButton.visible = true;
    taskView.itemId = -1;

    updateContextActionsStates();
  }

  function updateContextActionsStates()
  {
    if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected == 0) { // root is selected
      taskActions.showOnlyCategory("home")
    } else if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected != 0) { // top-level is selected
      taskActions.showOnlyCategory("account")
    } else if ( collectionView.numSelected > 1 ) {
      taskActions.showOnlyCategory( "multiple_folder" );
    } else {
      taskActions.showOnlyCategory("single_folder")
    }
  }

  KCal.IncidenceView {
    id: taskView
    anchors { fill: parent; topMargin: 48; leftMargin: 48 }
    width: parent.width
    height: parent.height
    visible: false

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

    KPIM.Button {
      anchors.bottom: backButton.top
      anchors.right: parent.right
      anchors.margins: 12
      width: 70
      height: 70
      icon: KDE.locate( "data", "mobileui/edit-button.png" );
      onClicked: {
        application.editIncidence( parent.item );
        eventView.visible = false;
        agendaView.visible = true;
      }
    }
    KPIM.Button {
      id: backButton
      anchors.bottom: parent.bottom
      anchors.right: parent.right
      anchors.margins: 12
      width: 70
      height: 70
      icon: KDE.locate( "data", "mobileui/back-to-list-button.png" );
      onClicked: {
        goBackToListing();
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

    QML.Image {
      id: backgroundImage
      x: 0
      y: 0
// FIXME: too big, costs about 1.5Mb RAM
//      source: "tasks-mobile-background.png"
      visible: collectionView.visible
    }

    Akonadi.AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      width: 1/3 * parent.width
      anchors.bottom : selectButton.top
      //height : parent.height - ( collectionView.hasSelection ? 0 : selectButton.height)
      anchors.left: parent.left

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      multipleSelectionText : KDE.i18n("You have selected \n%1 folders\nfrom %2 accounts\n%3 tasks", collectionView.numSelected,
                                                                                                        application.numSelectedAccounts,
                                                                                                        itemList.count)
      QML.Component.onCompleted : updateContextActionsStates();
      onNumBreadcrumbsChanged : updateContextActionsStates();
      onNumSelectedChanged : updateContextActionsStates();
      onSelectedClicked : {
        mainWorkView.visible = false
        bulkActionScreen.visible = true
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
            buttonText : KDE.i18n( "New Task" )
            onClicked: { application.newTask(); actionPanel.collapse() }
          }
        }
      ]
    }

    QML.Rectangle {
      id : accountPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : (collectionView.hasSelection && !collectionView.hasBreadcrumbs) && (itemList.count == 0) ? 1 : 0


      KPIM.Button2 {
        anchors.top : parent.top
        anchors.topMargin : 30
        anchors.left : parent.left
        anchors.right : parent.right
        anchors.leftMargin : 10
        anchors.rightMargin : 10
        buttonText : KDE.i18n( "New Task" )
        onClicked : {
          application.newTask();
        }
      }
    }

    QML.Rectangle {
      id : emptyFolderPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : (collectionView.hasBreadcrumbs && itemList.count == 0 ) ? 1 : 0
      // TODO: content
      QML.Text {
        text : KDE.i18n("No tasks in this folder");
        height : 20;
        font.italic : true
        horizontalAlignment : QML.Text.AlignHCenter
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
    }

    QML.Rectangle {
      id : itemListPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : itemList.count > 0 ? 1 : 0

      TaskListView {
        id: itemList
        model: itemModel
        checkModel : _itemActionModel
        anchors.fill: parent
        onItemSelected: {
          taskView.itemId = itemList.currentItemId;
          taskView.visible = true;
          taskActions.showOnlyCategory("todo_viewer")
          collectionView.visible = false;
          itemListPage.visible = false;
          selectButton.visible = false;
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      id: actionPanelNew
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      content : [
        TaskActions {
          id : taskActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanelNew.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanelNew.collapse();
                favoriteSelector.visible = true;
                mainWorkView.visible = false;
              }
            },
            KPIM.ScriptAction {
              name : "add_as_favorite"
              script : {
                actionPanelNew.collapse();
                application.saveFavorite();
              }
            },
            KPIM.ScriptAction {
              name : "start_maintenance"
              script : {
                actionPanelNew.collapse();
                mainWorkView.visible = false
                bulkActionScreen.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "edit_todo"
              script : {
                application.editIncidence( taskView.item );
                eventView.visible = false;
                agendaView.visible = true;
                actionPanelNew.collapse();
              }
            }
          ]

          onTriggered : {
            console.log("Triggered was: " + triggeredName)
          }
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      handlePosition : actionPanelNew.handlePosition + actionPanelNew.handleHeight
      id: attachmentPanel
      visible: taskView.attachmentModel.attachmentCount >= 1
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - startPanel.handlePosition - startPanel.handleHeight - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: taskView.attachmentModel
          anchors.fill: parent
        }
      ]
    }
  }

  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    anchors.fill : parent
    visible : false
    backgroundImage : backgroundImage.source
    onFinished : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
      application.clearPersistedSelection("preFavSelection");
      application.multipleSelectionFinished();
    }
    onCanceled : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
      application.restorePersistedSelection("preFavSelection");
    }
  }
  KPIM.BulkActionScreen {
    id : bulkActionScreen
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right
    backgroundImage : backgroundImage.source

    visible : false
    actionListWidth : 1/3 * parent.width
    multipleText : KDE.i18n("%1 folders", collectionView.numSelected)
    selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
    headerList : TaskListView {
      showCheckBox : true
      id: bulkActionHeaderList
      model: itemModel
      checkModel : _itemActionModel
      anchors.fill : parent
      showCompletionSlider: false
    }
    onBackClicked : {
      bulkActionScreen.visible = false
      mainWorkView.visible = true
    }
  }

  QML.Connections {
    target: taskView
    onIncidenceRemoved : { goBackToListing(); }
  }

  QML.Connections {
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
    }
  }

  KPIM.AboutDialog {
    id : aboutDialog
    source: backgroundImage.source
  }
}
