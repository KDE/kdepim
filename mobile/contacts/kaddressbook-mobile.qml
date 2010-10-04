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
import org.kde.akonadi.contacts 4.5 as Contacts

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
    selectButton.visible = true;
    contactView.itemId = -1;
    contactGroupView.itemId = -1;

    updateContextActionsStates();
  }

  function updateContextActionsStates()
  {
    if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected == 0) { // root is selected
      kaddressbookActions.showOnlyCategory("home")
      application.setScreenVisibilityState( 0 )
    } else if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected != 0) { // top-level is selected
      kaddressbookActions.showOnlyCategory("account")
      application.setScreenVisibilityState( 1 )
    } else if ( collectionView.numSelected > 1 ) {
      kaddressbookActions.showOnlyCategory( "multiple_folder" );
      application.setScreenVisibilityState( 2 )
    } else {
      kaddressbookActions.showOnlyCategory("single_folder")
      application.setScreenVisibilityState( 4 )
    }
  }

  Contacts.ContactView {
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

  Contacts.ContactGroupView {
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
//      source: "kaddressbook-mobile-background.png"
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

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 address books, %2 is e.g. from 2 accounts, %3 is e.g. 9 contacts",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 address book","%1 address books",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 contact","%1 contacts",contactList.count))


      QML.Component.onCompleted : updateContextActionsStates();
      onNumBreadcrumbsChanged : updateContextActionsStates();
      onNumSelectedChanged : updateContextActionsStates();

      onSelectedClicked : {
        mainWorkView.visible = false
        bulkActionScreen.visible = true
        application.isBulkActionScreenSelected = true
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
          height : startPageNewContactButton.height + startPageNewContactGroupButton.height + 3 * spacing
          KPIM.Button2 {
            id: startPageNewContactButton
            width: parent.width
            buttonText : KDE.i18n( "New Contact" )
            onClicked : {
              application.newContact();
            }
          }
          KPIM.Button2 {
            id: startPageNewContactGroupButton
            width: parent.width
            buttonText : KDE.i18n( "New Contact Group" )
            onClicked : {
              application.newContactGroup();
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
      opacity : application.isHomeScreenVisible ? 1 : 0


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
        id : newContactGroupButton
        anchors.top : newContactButton.bottom
        anchors.left : parent.left
        anchors.right : parent.right
        anchors.leftMargin : 10
        anchors.rightMargin : 10
        buttonText : KDE.i18n( "New Contact Group" )
        onClicked : {
          application.newContactGroup();
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
      opacity : application.isHomeScreenVisible ? 0 : 1

      Akonadi.FilterLineEdit {
        id: filterLineEdit
        anchors.left : parent.left
        anchors.top : parent.top
        anchors.bottom : contactList.top
        anchors.right : parent.right
        visible : false
        height : 0
      }

      ContactListView {
        id: contactList
        anchors.left : parent.left
        anchors.top : filterLineEdit.bottom
        anchors.bottom : parent.bottom
        anchors.right : parent.right
        model: itemModel
        checkModel : _itemActionModel
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
            selectButton.visible = false;
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
            selectButton.visible = false;
            kaddressbookActions.showOnlyCategory("contact_viewer")
          }
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
        KAddressBookActions {
          id : kaddressbookActions
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
                application.isBulkActionScreenSelected = true
              }
            }
          ]

          onDoCollapse : actionPanelNew.collapse();

          onTriggered : {
            console.log("Triggered was: " + triggeredName)
          }
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
    multipleText : KDE.i18np("1 folder", "%1 folders", collectionView.numSelected)
    selectedItemModel : _breadcrumbNavigationFactory.qmlSelectedItemModel();
    headerList : ContactListView {
      showCheckBox : true
      id: bulkActionContactList
      model: itemModel
      checkModel : _itemActionModel
      anchors.fill : parent
    }
    onBackClicked : {
      bulkActionScreen.visible = false
      application.isBulkActionScreenSelected = false
      mainWorkView.visible = true
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
    target: startPage
    onFavoriteSelected : {
      application.loadFavorite(favName);
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

  KPIM.AboutDialog {
    id : aboutDialog
    source: backgroundImage.source
  }
}
