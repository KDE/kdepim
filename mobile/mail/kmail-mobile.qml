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
import org.kde.messageviewer 4.5
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.MainView {
  id: kmailMobile

  SystemPalette { id: palette; colorGroup: "Active" }

  MessageView {
    id: messageView
    z: 0
    anchors.left: parent.left
    width: parent.width
    height: parent.height
    itemId: -1
    swipeLength: 0.2 // Require at least 20% of screenwidth to trigger next or prev

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


  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "kmail-mobile", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: KDE.i18n( "Mail start page" )
          favoritesModel : favoritesList

          contextActions: [
            Column {
              anchors.fill: parent
              height: 480 / 6 * 3
              KPIM.Button {
                width: parent.width
                height: 480 / 6
                buttonText : KDE.i18n( "Write new Email" )
                font.bold: true
                onClicked : {
                  startPanel.collapse();
                  application.startComposer();
                }
              }
              KPIM.Button {
                width: parent.width
                height: 480 / 6
                buttonText : KDE.i18n( "Add Account" )
                font.bold: true
                onClicked : { application.launchAccountWizard(); }
              }
              KPIM.Button {
                height : 480 / 6
                width : parent.width
                buttonText : KDE.i18n( "Add Favorite" )
                font.bold:  true
                onClicked : { favoriteSelector.visible = true; startPage.visible = false; }
              }
            }
          ]
        },
        FavoriteSelector {
          id : favoriteSelector
          anchors.fill : parent
          visible : false
          onCanceled: {
            favoriteSelector.visible = false;
            startPage.visible = true;
          }
          onFinished : {
            favoriteSelector.visible = false;
            startPage.visible = true;
            application.saveFavorite( favoriteSelector.favoriteName );
          }
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: folderPanel
      titleText: KDE.i18n( "Folders" )
      handleHeight: 150
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

          HeaderView {
            id: headerList
            opacity : { headerList.count > 0 ? 1 : 0; }
            model: itemModel
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: collectionView.right
            onItemSelected: {
              // Prevent reloading of the message, perhaps this should be done
              // in messageview itself.
              if ( messageView.itemId != headerList.currentItemId )
              {
                messageView.messagePath = application.pathToItem(headerList.currentItemId);
                messageView.itemId = headerList.currentItemId;
              }
              folderPanel.collapse()
            }
          }
          Rectangle {
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
          }
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
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
      handleHeight: parent.height - startPanel.handlePosition - startPanel.handleHeight - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
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

  Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
      startPanel.collapse();
      folderPanel.expand();
    }

  }

  Connections {
    target: collectionView
    onChildCollectionSelected : { application.setSelectedChildCollectionRow(row); }
  }

  Connections {
    target: collectionView
    onBreadcrumbCollectionSelected : { application.setSelectedBreadcrumbCollectionRow(row); }
  }

}
