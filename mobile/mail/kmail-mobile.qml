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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.akonadi 4.5 as Akonadi
import org.kde.messageviewer 4.5 as MessageViewer
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.MainView {
  id: kmailMobile


  QML.SystemPalette { id: palette; colorGroup: "Active" }

  MessageViewer.MessageView {
    id: messageView
    z: 0
    anchors.left: parent.left
    width: parent.width
    height: parent.height
    itemId: -1
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev
    visible : false
    onNextItemRequest: {
      // Only go to the next message when currently a valid item is set.
      if ( messageView.itemId >= 0 )
        headerList.nextItem();
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( messageView.itemId >= 0 )
        headerList.previousItem();
    }
  }

  QML.Rectangle {
    id : backToMessageListButton
    y : 200
    width : 32
    height : 32
    color : "blue"
    visible : false
    QML.Image {
      source : KDE.iconPath("draw-arrow-back", 32)
    }
    QML.MouseArea {
      anchors.fill : parent;
      onClicked : {
        messageView.visible = false;
        backToMessageListButton.visible = false;
        collectionView.visible = true;
        emailListPage.visible = true;
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
      breadcrumbItemsModel : breadcrumbCollectionsModel
      selectedItemModel : selectedCollectionModel
      childItemsModel : childCollectionsModel
    }
    KPIM.Button2 {
      id : selectButton
      anchors.left: collectionView.left
      anchors.right: collectionView.right
      anchors.bottom : parent.bottom
      anchors.bottomMargin : collectionView.hasSelection ? -selectButton.height : 0
      buttonText : KDE.i18n("Select")
      opacity : collectionView.hasSelection ? 0 : 1
      onClicked : {
        favoriteSelector.visible = true;
        mainWorkView.visible = false;

        // show select multiple dialog
        // Show selection content.
        // Change contents of this panel.

        // When Home is clicked do the reverse.
      }

    }
    KPIM.StartCanvas {
      id : startPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      anchors.leftMargin : 50

      opacity : collectionView.hasSelection ? 0 : 1
      showAccountsList : false
      favoritesModel : favoritesList

      contextActions : [
        QML.Column {
          anchors.fill: parent
          height : 70
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "Write new Email" )
            onClicked : {
              application.startComposer();
            }
          }
        }
      ]
    }

    QML.Rectangle {
      id : homePage
    }
    QML.Rectangle {
      id : accountPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : (!collectionView.hasSelection || headerList.count) > 0 ? 0 : 1
      // TODO: content
      QML.Text {
        text : "This space intentionally left blank";
        height : 20;
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
      /*
      QML.Rectangle {
        id : headerActionOverlay
        color: "#00000000" // Set a transparant color.
        opacity : { headerList.count > 0 ? 0 : 1; }
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: collectionView.right
        KPIM.Button {
          id : newEmailButton
          anchors.top : parent.top
          anchors.left : parent.left
          anchors.right : parent.right
          height : 30
          buttonText : KDE.i18n( "Write new E-Mail" )
          onClicked : {
            folderPanel.collapse();
            application.startComposer();
          }
        }
        KPIM.Button {
          id : searchEmailButton
          anchors.top : newEmailButton.bottom
          anchors.left : parent.left
          anchors.right : parent.right
          height : 30
          buttonText : KDE.i18n( "Search for E-Mail" )
          onClicked : {
            console.log("Search email");
          }
        }
        KPIM.Button {
          anchors.top : searchEmailButton.bottom
          anchors.left : parent.left
          anchors.right : parent.right
          height : 30
          buttonText : KDE.i18n( "Configure Account" )
          onClicked : {
            console.log("Configure");
          }
        }
      }*/
    }

    QML.Rectangle {
      id : emailListPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : headerList.count > 0 ? 1 : 0

      HeaderView {
        id: headerList
        model: itemModel
        anchors.fill : parent
        onItemSelected: {
          // Prevent reloading of the message, perhaps this should be done
          // in messageview itself.
          if ( messageView.itemId != headerList.currentItemId )
          {
            messageView.messagePath = application.pathToItem(headerList.currentItemId);
            messageView.itemId = headerList.currentItemId;
            messageView.visible = true;
            backToMessageListButton.visible = true;
            collectionView.visible = false;
            emailListPage.visible = false;
          }
        }
      }
    }
  }
  Akonadi.FavoriteSelector {
    id : favoriteSelector
    anchors.fill : parent
    visible : false
    onCanceled: {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
    }
    onFinished : {
      favoriteSelector.visible = false;
      mainWorkView.visible = true;
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    SlideoutPanel {

      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 300
      handleHeight: 150
      contentWidth: 240
      content: [
          KPIM.Button {
            id: replyButton
            anchors.top: parent.top;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Reply" )
            onClicked: {
              actionPanel.collapse();
              application.reply( messageView.itemId );
            }
          },
          KPIM.Button {
            id: replyAllButton
            anchors.top: replyButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Reply To All" )
            onClicked: {
              actionPanel.collapse();
              application.replyToAll( messageView.itemId );
            }
          },
          KPIM.Button {
            id: forwardButton
            anchors.top: replyAllButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Forward" )
            onClicked: {
              actionPanel.collapse();
              application.forwardInline( messageView.itemId );
            }
          },
          KPIM.Action{
            id : deleteButton
            anchors.top: forwardButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            hardcoded_height: parent.height / 6
            action : application.getAction("akonadi_item_delete")
            onTriggered : actionPanel.collapse();
          },
          KPIM.Action{
            id : importantButton
            anchors.top: deleteButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            hardcoded_height: parent.height / 6
            action : application.getAction("mark_message_important")
            checkable : true
            onTriggered : actionPanel.collapse();
          },
          KPIM.Action{
            id : markAsActionButton
            anchors.top: importantButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            hardcoded_height: parent.height / 6
            action : application.getAction("mark_message_action_item")
            checkable : true
            onTriggered : actionPanel.collapse();
          }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: attachmentPanel
      visible: messageView.attachmentModel.attachmentCount >= 1
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight : 70
//       handleHeight: parent.height - startPanel.handlePosition - startPanel.handleHeight - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      contentWidth: attachmentView.requestedWidth
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: messageView.attachmentModel
          anchors.fill: parent
        }
      ]
    }
  }

  QML.Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
      startPanel.collapse();
      folderPanel.expand();
    }
  }

  QML.Connections {
    target: collectionView
    onChildCollectionSelected : { application.setSelectedChildCollectionRow(row); }
  }

  QML.Connections {
    target: collectionView
    onBreadcrumbCollectionSelected : { application.setSelectedBreadcrumbCollectionRow(row); }
  }

}
