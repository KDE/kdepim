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

  QML.Connections {
    target: guiStateManager
    onGuiStateChanged: { updateContextActionStates() }
  }

  QML.Component.onCompleted : updateContextActionStates();

  function updateContextActionStates()
  {
    if ( guiStateManager.inHomeScreenState ) {
      taskActions.showOnlyCategory( "home" )
    } else if ( guiStateManager.inAccountScreenState ) {
      taskActions.showOnlyCategory( "account" )
    } else if ( guiStateManager.inSingleFolderScreenState ) {
      taskActions.showOnlyCategory( "single_folder" )
    } else if ( guiStateManager.inMultipleFolderScreenState ) {
      taskActions.showOnlyCategory( "multiple_folder" )
    } else if ( guiStateManager.inViewSingleItemState ) {
      taskActions.showOnlyCategory( "todo_viewer" )
    }
  }

  KCal.IncidenceView {
    id: taskView
    visible: guiStateManager.inViewSingleItemState
    anchors { fill: parent; topMargin: 48; leftMargin: 48 }
    width: parent.width
    height: parent.height

    z: 0

    itemId: -1
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

    onNextItemRequest: {
        _itemNavigationModel.requestNext();
    }

    onPreviousItemRequest: {
        _itemNavigationModel.requestPrevious();
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
        guiStateManager.popState();
      }
    }
  }

  QML.Item {
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

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 folders, %2 is e.g. from 2 accounts, %3 is e.g. 9 tasks",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 folder","%1 folders",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 task","%1 tasks",itemList.count))

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
        QML.Column {
          anchors.fill: parent
          height : 70
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "New Task" )
            onClicked : {
              application.newTask();
            }
          }
        }
      ]
    }

    QML.Rectangle {
      id : emptyFolderPage
      visible: (!guiStateManager.inHomeScreenState && collectionView.hasBreadcrumbs && itemList.count == 0)
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
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
      visible: { guiStateManager.inAccountScreenState ||
                 guiStateManager.inSingleFolderScreenState ||
                 guiStateManager.inMultipleFolderScreenState
               }
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"

      Akonadi.FilterLineEdit {
        id: filterLineEdit
        anchors.left : parent.left
        anchors.top : parent.top
        anchors.bottom : itemList.top
        anchors.right : parent.right
        visible : false
        height : 0
      }

      TaskListView {
        id: itemList
        model: itemModel
        checkModel : _itemActionModel
        anchors.left : parent.left
        anchors.top : filterLineEdit.bottom
        anchors.bottom : parent.bottom
        anchors.right : parent.right
        navigationModel : _itemNavigationModel
      }
      QML.Connections {
        target : _itemNavigationModel
        onCurrentRowChanged : {
          guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    visible: !guiStateManager.inBulkActionScreenState && !guiStateManager.inMultipleFolderSelectionScreenState

    SlideoutPanel {
      id: actionPanel
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
                actionPanel.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptAction {
              name : "configure"
              script : {
                actionPanel.collapse();
                configDialog.visible = true;
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState )
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
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState )
              }
            },
            KPIM.ScriptAction {
              name : "edit_todo"
              script : {
                application.editIncidence( taskView.item );
                actionPanel.collapse();
              }
            }
          ]

          onDoCollapse : actionPanel.collapse();
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      handlePosition : actionPanel.handlePosition + actionPanel.handleHeight
      id: attachmentPanel
      visible: (taskView.attachmentModel.attachmentCount >= 1) && guiStateManager.inViewSingleItemState
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - actionPanel.handlePosition - actionPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: taskView.attachmentModel
          anchors.fill: parent

          onOpenAttachment: {
            application.openAttachment(url, mimeType);
          }

          onSaveAttachment: {
            application.saveAttachment(url);
          }
        }
      ]
    }
  }

  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    visible : guiStateManager.inMultipleFolderSelectionScreenState
    anchors.fill : parent
    backgroundImage : backgroundImage.source
    onFinished : {
      guiStateManager.popState()
      application.clearPersistedSelection("preFavSelection");
      application.multipleSelectionFinished();
    }
    onCanceled : {
      guiStateManager.popState()
      application.restorePersistedSelection("preFavSelection");
    }
  }

  KPIM.BulkActionScreen {
    id : bulkActionScreen
    visible : guiStateManager.inBulkActionScreenState
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right
    backgroundImage : backgroundImage.source

    actionListWidth : 1/3 * parent.width
    multipleText : KDE.i18np("1 folder", "%1 folders", collectionView.numSelected)
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
      guiStateManager.popState()
    }
  }

  QML.Connections {
    target: taskView
    onIncidenceRemoved : { guiStateManager.popState(); }
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

  ConfigDialog {
    id: configDialog
    visible: false
  }
}
