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

  QML.Rectangle {
    id : editContactButton
    anchors.right : kaddressbookMobile.right
    anchors.rightMargin : 70
    anchors.bottom : backToFolderListButton.top
    anchors.bottomMargin : 70
    visible : false
    QML.Image {
      source : KDE.iconPath( "document-edit", 64 );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          application.editContact( contactView.item );
        }
      }
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

  QML.Rectangle {
    id : editContactGroupButton
    anchors.right : kaddressbookMobile.right
    anchors.rightMargin : 70
    anchors.bottom : backToFolderListButton.top
    anchors.bottomMargin : 70
    visible : false
    QML.Image {
      source : KDE.iconPath( "document-edit", 64 );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          application.editContactGroup( contactGroupView.item );
        }
      }
    }
  }

  QML.Rectangle {
    id : backToFolderListButton
    anchors.right : kaddressbookMobile.right
    anchors.rightMargin : 70
    anchors.bottom : kaddressbookMobile.bottom
    anchors.bottomMargin : 100
    visible : false
    QML.Image {
      source : "back-to-message-list.png"
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          contactView.visible = false;
          contactGroupView.visible = false;
          editContactButton.visible = false;
          editContactGroupButton.visible = false;
          backToFolderListButton.visible = false;
          collectionView.visible = true;
          contactListPage.visible = true;
          contactView.itemId = -1;
          contactGroupView.itemId = -1;
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

      multipleSelectionText : KDE.i18n("You have selected \n%1 addressbooks\nfrom %2 accounts\n%3 contacts", collectionView.numSelected,
                                                                                                        application.numSelectedAccounts,
                                                                                                        contactList.count)
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
          height : kaddressbookMobile.height 
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "New Contact" )
            onClicked : {
              application.newContact();
            }
          }
          KPIM.Button2 {
            width: parent.width
            buttonText : KDE.i18n( "New Contact Group" )
            onClicked : {
              application.newContactGroup();
            }
          }
          KPIM.Button2 {
            width: parent.width
            buttonText: KDE.i18n( "New Address Book" )
            onClicked : {
              application.launchAccountWizard();
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
      opacity : (collectionView.hasSelection && !collectionView.hasBreadcrumbs) && (contactList.count == 0) ? 1 : 0


      KPIM.Button2 {
        id : newContactButton
        anchors.top : parent.top
        anchors.topMargin : 30
        anchors.left : parent.left
        anchors.right : parent.right
        anchors.leftMargin : 10
        anchors.rightMargin : 10
        buttonText : KDE.i18n( "New Contact" )
        onClicked : {
          application.newContact();
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
      opacity : (collectionView.hasBreadcrumbs && contactList.count == 0 ) ? 1 : 0
      // TODO: content
      QML.Text {
        text : KDE.i18n("No contacts in this addressbook");
        height : 20;
        font.italic : true
        horizontalAlignment : QML.Text.AlignHCenter
        anchors.verticalCenter : parent.verticalCenter;
        anchors.horizontalCenter : parent.horizontalCenter
      }
    }

    QML.Rectangle {
      id : contactListPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : contactList.count > 0 ? 1 : 0

      ContactListView {
        id: contactList
        model: itemModel
        anchors.fill : parent
        onItemSelected: {
          if ( itemModel.typeForIndex( contactList.currentIndex ) == "contact" ) {
            contactView.itemId = contactList.currentItemId;
            contactView.visible = true;
            contactGroupView.visible = false;
            editContactButton.visible = true;
            editContactGroupButton.visible = false;
            backToFolderListButton.visible = true;
            collectionView.visible = false;
            contactListPage.visible = false;
          }
          if ( itemModel.typeForIndex( contactList.currentIndex ) == "group" ) {
            contactGroupView.itemId = contactList.currentItemId;
            contactView.visible = false;
            contactGroupView.visible = true;
            editContactButton.visible = false;
            editContactGroupButton.visible = true;
            backToFolderListButton.visible = true;
            collectionView.visible = false;
            contactListPage.visible = false;
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

    SlideoutPanel {
      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      handlePosition: 225
      contentWidth: 240
      content: [
          KPIM.Action {
            id: deleteButton
            anchors.top: parent.top;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            action : application.getAction("akonadi_item_delete")
            onTriggered: actionPanel.collapse();
          },
          KPIM.Button {
            id : saveFavoriteButton
            anchors.top: deleteButton.bottom;
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
            id : newContactButton2
            anchors.top: saveFavoriteButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText : KDE.i18n( "New Contact" )
            onClicked : {
              application.newContact();
              actionPanel.collapse();
            }
          },
          KPIM.Button {
            id : newContactGroupButton
            anchors.top: newContactButton2.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText : KDE.i18n( "New Group" )
            onClicked : {
              application.newContactGroup();
              actionPanel.collapse();
            }
          },
          KPIM.Button {
            visible : !collectionView.hasSelection
            anchors.top: newContactGroupButton.bottom;
            anchors.horizontalCenter: parent.horizontalCenter;
            width: parent.width - 10
            height: parent.height / 6
            buttonText : KDE.i18n( "New Address Book" )
            onClicked : {
              application.launchAccountWizard();
              actionPanel.collapse();
            }
          }
      ]
    }
  }

  QML.Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
      // TODO: Figure out how to expand the slider programatically.
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
