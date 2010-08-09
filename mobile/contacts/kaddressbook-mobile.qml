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
import org.kde.akonadi.contacts 4.5 as Akonadi

KPIM.MainView {
  id: kaddressbookMobile

  function goBackToListing()
  {
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

  function updateContextActionsStates()
  {
    if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected == 0) // root is selected
    {
      kaddressbookActions.showOnlyCategory("home")
    } else if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected != 0) // top-level is selected
    {
      kaddressbookActions.showOnlyCategory("resource")
    } else { // something else is selected
      kaddressbookActions.showOnlyCategory("single_folder")
    }
  }

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

  KPIM.Button {
    id : editContactButton
    anchors.bottom: backToFolderListButton.top
    anchors.right: parent.right
    anchors.margins: 12
    width: 70
    height: 70
    visible : false
    icon: KDE.locate( "data", "mobileui/edit-button.png" );
    onClicked: {
      application.editContact( contactView.item );
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

  KPIM.Button {
    id : editContactGroupButton
    anchors.bottom: backToFolderListButton.top
    anchors.right: parent.right
    anchors.margins: 12
    width: 70
    height: 70
    visible : false
    icon: KDE.locate( "data", "mobileui/edit-button.png" );
    onClicked: {
      application.editContactGroup( contactGroupView.item );
    }
  }

  KPIM.Button {
    id : backToFolderListButton
    anchors.bottom: kaddressbookMobile.bottom
    anchors.right: kaddressbookMobile.right
    anchors.margins: 12
    width: 70
    height: 70
    visible : false
    icon: KDE.locate( "data", "mobileui/back-to-list-button.png" );
    onClicked: {
      goBackToListing();
      kaddressbookActions.showOnlyCategory("resource")
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
      source: "kaddressbook-mobile-background.png"
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

      multipleSelectionText : KDE.i18n("You have selected \n%1 addressbooks\nfrom %2 accounts\n%3 contacts", collectionView.numSelected,
                                                                                                        application.numSelectedAccounts,
                                                                                                        contactList.count)
      QML.Component.onCompleted : updateContextActionsStates();
      onNumBreadcrumbsChanged : updateContextActionsStates();
      onNumSelectedChanged : updateContextActionsStates();
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
        text : KDE.i18n("No contacts in this address book");
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
            kaddressbookActions.showOnlyCategory("contact_viewer")
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
            kaddressbookActions.showOnlyCategory("contact_viewer")
          }
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
      id: actionPanelNew
      titleText: KDE.i18n( "Context Actions" )
      handlePosition : 125
      handleHeight: 150
      contentWidth: parent.width - 20
      anchors.fill : parent

      content : [
        KAddressBookActions {
          id : kaddressbookActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptActionItem {
              name : "show_about_dialog"
              script : {
                actionPanelNew.collapse();
                aboutDialog.visible = true
              }
            },
            KPIM.ScriptActionItem {
              name : "to_selection_screen"
              script : {
                actionPanelNew.collapse();
                favoriteSelector.visible = true;
                mainWorkView.visible = false;
              }
            }
          ]

          onTriggered : {
            console.log("Triggered was: " + triggeredName)
          }
        }
      ]
    }
  }
/*
  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      handlePosition: 125
      contentWidth: 240
      content: [
          KPIM.Action {
            id: syncButton
            anchors.top: parent.top;
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
*/
  QML.Connections {
    target: startPage
    onAccountSelected : {
      application.setSelectedAccount(row);
      // TODO: Figure out how to expand the slider programatically.
    }
  }

  QML.Connections {
    target: contactView
    onContactRemoved : { goBackToListing(); }
  }

  QML.Connections {
    target: contactGroupView
    onContactGroupRemoved : { goBackToListing(); }
  }
}
