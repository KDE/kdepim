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
      titleIcon: KDE.iconPath( "kmail", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50

          contextActions : [
            KPIM.Button {
              id : start_newEmailButton
              height : 20
              width : 200
              anchors.top : parent.top
              buttonText : "Write new Email"
              onClicked : {
                console.log( "Write new clicked" );
              }

            },
            KPIM.Button {
              id : start_newAccountButton
              anchors.top : start_newEmailButton.bottom
              height : 20
              width : 200
              buttonText : "Add Account"
              onClicked : {
                console.log( "Add Account clicked" );
                application.launchAccountWizard();
              }
            }
          ]
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: folderPanel
      titleText: "Folders"
      handleHeight: 150
      handlePosition: startPanel.handlePosition + startPanel.handleHeight
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
                messageView.itemId = headerList.currentItemId;
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
              buttonText : "Write new E-Mail"
              onClicked : {
                console.log("Write new");
              }
            }
            KPIM.Button {
              id : searchEmailButton
              anchors.top : newEmailButton.bottom
              anchors.left : parent.left
              anchors.right : parent.right
              height : 30
              buttonText : "Search for E-Mail"
              onClicked : {
                console.log("Search email");
              }
            }
            KPIM.Button {
              anchors.top : searchEmailButton.bottom
              anchors.left : parent.left
              anchors.right : parent.right
              height : 30
              buttonText : "Configure Account"
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
      titleText: "Actions"
      // ### QML has a bug where the children property is broken.
      // As a workaround, we need to set handlePosition here and
      // set anchors.fill parent on the panels. Remove when Qt is fixed.
      handlePosition: folderPanel.handlePosition + folderPanel.handleHeight
      handleHeight: 150
      contentWidth: 240
      content: [
          KPIM.Button {
            id: moveButton
            anchors.top: parent.top;
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
              if ( messageView.itemId >= 0 )
                headerList.previousItem();

              actionPanel.collapse();
            }
          },
          KPIM.Button {
            anchors.top: previousButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: "Next"
            onClicked: {
              if ( messageView.itemId >= 0 )
                headerList.nextItem();

              actionPanel.collapse();
            }
          }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      handlePosition: actionPanel.handlePosition + actionPanel.handleHeight
      id: attachmentPanel
      visible: messageView.attachmentModel.attachmentCount >= 1
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: parent.height - startPanel.handlePosition - startPanel.handleHeight - actionPanel.handleHeight - folderPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      contentWidth: attachmentView.requestedWidth
      content: [
        AttachmentList {
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
      // TODO: Figure out how to expand the slider programatically.
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
