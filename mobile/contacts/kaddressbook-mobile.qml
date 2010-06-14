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
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi.contacts 4.5 as Akonadi

KPIM.MainView {
  id: kaddressbookMobile

  Akonadi.ContactView {
    id: contactView
    z: 0
    anchors.fill: parent
    itemId: -1

    onNextItemRequest: {
      // Only go to the next message when currently a valid item is set.
      if ( contactView.itemId >= 0 )
        contactList.nextItem();
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( contactView.itemId >= 0 )
        contactList.previousItem();
    }
}
  Akonadi.ContactGroupView {
    id: contactGroupView
    z: 0
    anchors.fill: parent
    itemId: -1
    visible: false

    onNextItemRequest: {
      // Only go to the next message when currently a valid item is set.
      if ( contactGroupView.itemId >= 0 )
        contactList.nextItem();
    }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( contactGroupView.itemId >= 0 )
        contactList.previousItem();
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: startPanel
      titleIcon: KDE.iconPath( "view-pim-contacts", 48 )
      handlePosition: 30
      handleHeight: 78
      content: [
        KPIM.StartCanvas {
          id : startPage
          anchors.fill : parent
          anchors.leftMargin : 50
          startText: KDE.i18n( "Address book start page" )
          favoritesModel : favoritesList

          contextActions : [
            Flickable { // Use a flickable because we've more than 3 actions.
              clip: true
              anchors.fill: parent
              height: 480 / 6 * 3
              contentWidth: width
              contentHeight: 480 / 6 * 4
              Column {
                anchors.fill: parent
                KPIM.Button {
                  height : 480 / 6
                  width : parent.width - 75
                  buttonText : KDE.i18n( "New Contact" )
                  onClicked : { application.newContact(); }
                }
                KPIM.Button {
                  height : 480 / 6
                  width : parent.width - 75
                  buttonText : KDE.i18n( "New Contact Group" )
                  onClicked : { console.log( "New Group clicked" ); }

                }
                KPIM.Button {
                  height: 480 / 6
                  width : parent.width - 75
                  buttonText : KDE.i18n( "Add Account" )
                  onClicked : { application.launchAccountWizard(); }
                }
                KPIM.Button {
                  height: 480 / 6
                  width : parent.width - 75
                  buttonText : KDE.i18n( "Favorites" )
                  onClicked : { favoriteSelector.visible = true; startPage.visible = false; }
                }
              }
            }
          ]
        },
        FavoriteSelector {
          id : favoriteSelector
          anchors.fill : parent
          visible : false
          styleSheet: window.styleSheet
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
      titleText: KDE.i18n( "Address books" )
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

          ContactListView {
            id: contactList
            opacity : { contactList.count > 0 ? 1 : 0; }
            model: itemModel
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: collectionView.right
            onItemSelected: {
              if ( itemModel.typeForIndex( contactList.currentIndex ) == "contact" ) {
                contactView.itemId = contactList.currentItemId;
                contactView.visible = true;
                contactGroupView.visible = false;
              }
              if ( itemModel.typeForIndex( contactList.currentIndex ) == "group" ) {
                contactGroupView.itemId = contactList.currentItemId;
                contactView.visible = false;
                contactGroupView.visible = true;
              }
              folderPanel.collapse()
            }
          }
          Item {
            id : headerActionOverlay
            opacity : { contactList.count > 0 ? 0 : 1; }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: collectionView.right
            KPIM.Button2 {
              id : newContactButton
              anchors.top : parent.top
              anchors.left : parent.left
              anchors.right : parent.right
              buttonText : KDE.i18n( "Add Contact" )
              onClicked : {
                application.newContact();
              }
            }
            /*KPIM.Button2 {
              id : searchEmailButton
              anchors.top : newEmailButton.bottom
              anchors.left : parent.left
              anchors.right : parent.right
              buttonText : KDE.i18n( "Search for Contact" )
              onClicked : {
                console.log("Search email");
              }
            }
            KPIM.Button2 {
              anchors.top : searchEmailButton.bottom
              anchors.left : parent.left
              anchors.right : parent.right
              buttonText : KDE.i18n( "Configure Account" )
              onClicked : {
                console.log("Configure");
              }
            }*/
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
            id: moveButton
            anchors.top: parent.top;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Move" )
            onClicked: actionPanel.collapse();
          },
          KPIM.Action {
            id: deleteButton
            anchors.top: moveButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            action : application.getAction("akonadi_item_delete")
            onTriggered: actionPanel.collapse();
          },
          KPIM.Button {
            id: previousButton
            anchors.top: deleteButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Previous" )
            onClicked: {
              if ( contactView.itemId >= 0 )
                contactList.previousItem();

              actionPanel.collapse();
            }
          },
          KPIM.Button {
            anchors.top: previousButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText: KDE.i18n( "Next" )
            onClicked: {
              if ( contactView.itemId >= 0 )
                contactList.nextItem();

              actionPanel.collapse();
            }
          }
      ]
    }
  }

  Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
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
