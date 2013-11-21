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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.akonadi 4.5 as Akonadi
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi.contacts 4.5 as Contacts
import "../mobileui/ScreenFunctions.js" as Screen

KPIM.MainView {
  id: kaddressbookMobile

  QML.Connections {
    target: guiStateManager
    onGuiStateChanged: { updateContextActionStates() }
  }

  QML.Component.onCompleted : updateContextActionStates();

  function updateContextActionStates()
  {
    if ( guiStateManager.inHomeScreenState ) {
      kaddressbookActions.showOnlyCategory( "home" )
    } else if ( guiStateManager.inAccountScreenState ) {
      kaddressbookActions.showOnlyCategory( "account" )
    } else if ( guiStateManager.inSingleFolderScreenState ) {
      kaddressbookActions.showOnlyCategory( "single_folder" )
    } else if ( guiStateManager.inMultipleFolderScreenState || guiStateManager.inSearchResultScreenState ) {
      kaddressbookActions.showOnlyCategory( "multiple_folder" )
    } else if ( guiStateManager.inViewContactState || guiStateManager.inViewContactGroupState ) {
      kaddressbookActions.showOnlyCategory( "contact_viewer" )
    }
  }

  Contacts.ContactView {
    id: contactView
    visible: guiStateManager.inViewContactState
    anchors.fill: parent
    itemId: -1

    onNextItemRequest: {
      _itemNavigationModel.requestNext()
    }

    onPreviousItemRequest: {
      _itemNavigationModel.requestPrevious()
    }

  }

  KPIM.ItemEditButton {
    id: editContactButton
    visible: guiStateManager.inViewContactState
    actionName: "akonadi_contact_item_edit"
    anchors.bottom: backToFolderListButton.top
    anchors.right: parent.right
    anchors.margins: 12
    onClicked: {
      application.editContact( contactView.item );
    }
  }

  Contacts.ContactGroupView {
    id: contactGroupView
    visible: guiStateManager.inViewContactGroupState
    anchors.fill: parent
    itemId: -1

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

  KPIM.ItemEditButton {
    id: editContactGroupButton
    visible: guiStateManager.inViewContactGroupState
    actionName: "akonadi_contact_item_edit"
    anchors.bottom: backToFolderListButton.top
    anchors.right: parent.right
    anchors.margins: 12
    onClicked: {
      application.editContactGroup( contactGroupView.item );
    }
  }

  KPIM.Button {
    id : backToFolderListButton
    visible : guiStateManager.inViewContactState || guiStateManager.inViewContactGroupState
    anchors.bottom: kaddressbookMobile.bottom
    anchors.right: kaddressbookMobile.right
    anchors.margins: 12
    width: 70
    height: 70
    icon: KDE.locate( "data", "mobileui/back-to-list-button.png" );
    onClicked: {
      _itemActionModel.select(-1, 1)
      _itemNavigationModel.select(-1, 1)
      guiStateManager.popState();
    }
  }

  QML.Item {
    id : mainWorkView
    visible: { guiStateManager.inHomeScreenState ||
               guiStateManager.inAccountScreenState ||
               guiStateManager.inSingleFolderScreenState ||
               guiStateManager.inMultipleFolderScreenState }
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
      id: collectionView
      visible: { guiStateManager.inHomeScreenState ||
                 guiStateManager.inAccountScreenState ||
                 guiStateManager.inSingleFolderScreenState ||
                 guiStateManager.inMultipleFolderScreenState }
      anchors.top: parent.top
      width: 1/3 * parent.width
      anchors.bottom : selectButton.top
      //height : parent.height - ( collectionView.hasSelection ? 0 : selectButton.height)
      anchors.left: parent.left
      itemHeight: Screen.partition( height, 7 )

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 address books, %2 is e.g. from 2 accounts, %3 is e.g. 9 contacts",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 address book","%1 address books",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 contact","%1 contacts",contactList.count))

      onSelectedClicked : {
        guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
      }

      KPIM.AgentStatusIndicator {
        anchors { top: parent.top; right: parent.right; rightMargin: 10; topMargin: 10 }
      }
    }

    KPIM.Button2 {
      id: selectButton
      visible: guiStateManager.inHomeScreenState
      anchors.left: collectionView.left
      anchors.right: collectionView.right
      anchors.bottom : parent.bottom
      anchors.bottomMargin : { (collectionView.numSelected == 1) ? -selectButton.height : 0 }
      buttonText : (collectionView.numSelected <= 1) ? KDE.i18n("Select") : KDE.i18n("Change Selection")
      onClicked : {
        application.persistCurrentSelection("preFavSelection");
        guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
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
      id : emptyFolderPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      visible: (!guiStateManager.inHomeScreenState && collectionView.hasBreadcrumbs && contactList.count == 0)
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
      visible: { guiStateManager.inAccountScreenState ||
                 guiStateManager.inSingleFolderScreenState ||
                 guiStateManager.inMultipleFolderScreenState }

      ContactListView {
        id: contactList
        anchors.left : parent.left
        anchors.top : parent.top
        anchors.bottom : filterLineEdit.top
        anchors.right : parent.right
        model: itemModel
        checkModel : _itemActionModel
        itemHeight: Screen.partition( height, 7 )

        navigationModel : _itemNavigationModel
      }

      Akonadi.FilterLineEdit {
        id: filterLineEdit
        anchors.left : parent.left
        anchors.bottom : parent.bottom
        anchors.right : parent.right
        visible : false
        height : 0
        y : height == 0 ? parent.height : parent.height - height
      }
    }
  }
  QML.Connections {
    target : _itemNavigationModel
    onCurrentRowChanged : {
      contactList.currentRow = _itemNavigationModel.currentRow
      if ( itemModel.typeForIndex( _itemNavigationModel.currentRow ) == "contact" ) {
        contactView.itemId = _itemNavigationModel.currentItemIdHack;
      }
      if ( itemModel.typeForIndex( _itemNavigationModel.currentRow ) == "group" ) {
        contactGroupView.itemId = _itemNavigationModel.currentItemIdHack;
      }
      _itemActionModel.select( _itemNavigationModel.currentRow, 3 );
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 100

    visible: !guiStateManager.inBulkActionScreenState &&
             !guiStateManager.inMultipleFolderSelectionScreenState &&
             !guiStateManager.inConfigScreenState &&
             !guiStateManager.inSearchScreenState

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      QML.Component.onCompleted: {
        actionPanel.expanded.connect( kaddressbookActions, kaddressbookActions.refresh );
      }

      content : [
        KAddressBookActions {
          id : kaddressbookActions
          anchors.fill : parent

          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanel.collapse();
                aboutDialog.visible = true;
              }
            },
            KPIM.ScriptAction {
              name : "configure"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.ConfigScreenState );
              }
            },
//TODO enable when SearchWidget::query() is implemented
//             KPIM.ScriptAction {
//               name : "search_contact"
//               script : {
//                 actionPanel.collapse();
//                 guiStateManager.pushState( KPIM.GuiStateManager.SearchScreenState );
//               }
//             },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
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
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
              }
            }
          ]

          onDoCollapse : actionPanel.collapse();
        }
      ]
    }
  }

  QML.Loader {
    anchors.fill: parent
    source: guiStateManager.inMultipleFolderSelectionScreenState ? KDE.locate( "module", "imports/org/kde/pim/mobileui/MultipleSelectionScreen.qml" ) : ""
    onLoaded: { item.backgroundImage = backgroundImage.source; }
  }

  QML.Loader {
    anchors.fill: parent
    source: guiStateManager.inBulkActionScreenState ? "BulkActionComponent.qml" : ""
    onLoaded: {
      item.backgroundImage = backgroundImage.source
      item.itemHeight = Screen.partition( height, 7 )
    }
  }

  KPIM.SearchResultScreen {
    id : searchResultScreen
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right

    itemView: ContactListView {
      id: searchContactListView
      model: itemModel
      checkModel : _itemActionModel
      navigationModel : _itemNavigationModel
      anchors.fill : parent
      itemHeight: Screen.partition( height, 7 )
    }

    resultText: KDE.i18np( "One contact found", "%1 contacts found", searchContactListView.count )
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
    onContactRemoved : { guiStateManager.popState(); }
  }

  QML.Connections {
    target: contactGroupView
    onContactGroupRemoved : { guiStateManager.popState(); }
  }

  QML.Loader {
    id : aboutDialog
    anchors.fill: parent
    visible: false
    source: visible ? KDE.locate( "module", "imports/org/kde/pim/mobileui/AboutDialog.qml" ) : ""
    onLoaded: { item.backgroundSource = backgroundImage.source; }
  }

  QML.Loader {
    anchors.fill: parent
    source: guiStateManager.inConfigScreenState ? "ConfigDialog.qml" : ""
    onLoaded: item.load();
  }

  KPIM.SearchDialog {
    id: searchDialog
    searchWidget: Contacts.SearchWidget {
      anchors.fill: parent
    }
  }
}
