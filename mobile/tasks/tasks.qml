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
    taskView.itemId = -1;
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
      source: "tasks-mobile-background.png"
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
      KPIM.Button2 {
        anchors.bottom : parent.bottom
        anchors.bottomMargin : 10
        anchors.right : parent.right
        anchors.rightMargin : 35
        width : 230
        buttonText : KDE.i18n( "Configure Account" )
        onClicked : {
          application.configureCurrentAccount();
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

      KPIM.ItemListView {
        id: itemList
        delegate: [
          KPIM.ItemListViewDelegate {
            id : listDelegate
            height : itemListView.height / 7
            summaryContent: [
              QML.Text {
                id : summaryLabel
                anchors.top : parent.top
                anchors.topMargin : 1
                anchors.left : parent.left
                anchors.leftMargin : 10
                anchors.right: parent.right
                anchors.rightMargin: completionSlider.width
                text: KDE.i18n( "Task: %1", model.summary )
                color : "#0C55BB"
                font.pixelSize: 16
                elide: "ElideRight"
              },
              QML.Text {
                anchors.top : summaryLabel.bottom
                anchors.topMargin : 1
                anchors.left : parent.left
                anchors.leftMargin : 10
                anchors.right: parent.right
                anchors.rightMargin: completionSlider.width
                height : 30;
                text: KDE.i18n( "Details: %1", model.description )
                color: "#3B3B3B"
                font.pointSize: 14
                elide: "ElideRight"
              },
              KPIM.CompletionSlider {
                id: completionSlider
                anchors.top: parent.top
                anchors.right: parent.right
                percentComplete : model.percentComplete
                onPercentCompleteChanged : {
                  application.setPercentComplete(model.index, percentComplete);
                }
              },
              QML.Image {
                id : importantFlagImage
                anchors.verticalCenter : parent.verticalCenter;
                anchors.left : parent.left
                anchors.leftMargin : 15
                source : KDE.iconPath("emblem-important.png", parent.height + 16)
                opacity : model.is_important ? 0.25 : 0
              }
            ]
          }
        ]

        model: itemModel
        anchors.fill: parent
        onItemSelected: {
          taskView.itemId = itemList.currentItemId;
          taskView.visible = true;
          collectionView.visible = false;
          itemListPage.visible = false;
        }
      }
    }
  }
  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    anchors.fill : parent
    visible : false
    onFinished : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
      application.clearPersistedSelection("preFavSelection");
    }
    onCanceled : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
      application.restorePersistedSelection("preFavSelection");
    }
  }



  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      contentWidth: 240
      anchors.fill : parent
      content: [
          KPIM.Button {
            id: newTaskButton
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "New Task" )
            onClicked: {
              application.newTask();
              actionPanel.collapse();
            }
          },
          KPIM.Action {
            id: syncButton
            anchors.top: newTaskButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            hardcoded_height: parent.height / 6
            action : application.getAction("akonadi_collection_sync")
            onTriggered : actionPanel.collapse();
          },
          KPIM.Action {
             id: deleteButton
             anchors.top: syncButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             action : application.getAction("akonadi_item_delete")
             onTriggered : actionPanel.collapse();
           },
           KPIM.Button {
             id: previousButton
             anchors.top: deleteButton.bottom;
             anchors.horizontalCenter: parent.horizontalCenter;
             width: parent.width - 10
             height: parent.height / 6
             buttonText: KDE.i18n( "Previous" )
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
             buttonText: KDE.i18n( "Next" )
             onClicked: {
               itemList.nextItem();
               actionPanel.collapse();
             }
           }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      handlePosition : 150
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

   QML.Connections {
     target: taskView
     onIncidenceRemoved : { goBackToListing(); }
   }
}
