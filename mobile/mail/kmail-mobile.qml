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
    anchors.right : kmailMobile.right
    anchors.rightMargin : 70
    anchors.bottom : kmailMobile.bottom
    anchors.bottomMargin : 100
    visible : false
    QML.Image {
      source : "back-to-message-list.png"
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

      multipleSelectionText : KDE.i18na("You have selected \n%1 folders\nfrom %2 accounts\n%3 emails", [collectionView.numSelected,
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
            buttonText : KDE.i18n( "Write new Email" )
            onClicked : {
              application.startComposer();
            }
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
      opacity : (collectionView.hasSelection && !collectionView.hasBreadcrumbs) && (headerList.count == 0) ? 1 : 0


      KPIM.Button2 {
        id : newEmailButton
        anchors.top : parent.top
        anchors.topMargin : 30
        anchors.left : parent.left
        anchors.right : parent.right
        anchors.leftMargin : 10
        anchors.rightMargin : 10
        buttonText : KDE.i18n( "Write new Email" )
        onClicked : {
          application.startComposer();
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
      opacity : (collectionView.hasBreadcrumbs && headerList.count == 0 ) ? 1 : 0
      // TODO: content
      QML.Text {
        text : KDE.i18n("No messages in this folder");
        height : 20;
        font.italic : true
        horizontalAlignment : QML.Text.AlignHCenter
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
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
    styleSheet: window.styleSheet
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
    visible : !favoriteSelector.visible
    SlideoutPanel {

      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 225
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
          },
          KPIM.Button {
            id : saveFavoriteButton
            anchors.top: markAsActionButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            buttonText: KDE.i18n( "Save Favorite" )
            width: parent.width - 10
            height: collectionView.hasSelection ? parent.height / 6 : 0
            visible : collectionView.hasSelection
            onClicked : {
              application.saveFavorite();
              actionPanel.collapse();
            }
          },
          KPIM.Button {
            id : writeNewEmailButton
            anchors.top: saveFavoriteButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText : KDE.i18n( "Write new Email" )
            onClicked : {
              application.startComposer();
              actionPanel.collapse();
            }
          },
          KPIM.Button {
            visible : !collectionView.hasSelection
            anchors.top: writeNewEmailButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText : KDE.i18n( "New Account" )
            onClicked : {
              application.launchAccountWizard();
              actionPanel.collapse();
            }
          }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: attachmentPanel
      visible: messageView.attachmentModel.attachmentCount >= 1 && messageView.visible
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
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
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
