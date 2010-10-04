/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <amantia@kdab.com>

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

  function backToListing()
  {
    messageView.visible = false;
    backToMessageListButton.visible = false;
    collectionView.visible = true;
    emailListPage.visible = true;
    selectButton.visible = true;
    messageView.itemId = -1;

    updateContextActionsStates();
 }

  function updateContextActionsStates()
  {
    if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected == 0) { // root is selected
      kmailActions.showOnlyCategory("home")
      application.setScreenVisibilityState( 0 )
    } else if (collectionView.numBreadcrumbs == 0 && collectionView.numSelected != 0) { // top-level is selected
      kmailActions.showOnlyCategory("account")
      application.setScreenVisibilityState( 1 )
    } else if ( collectionView.numSelected > 1 ) {
      kmailActions.showOnlyCategory( "multiple_folder" );
      application.setScreenVisibilityState( 2 )
    } else {
      kmailActions.showOnlyCategory("single_folder")
      application.setScreenVisibilityState( 4 )
    }
  }


  QML.SystemPalette { id: palette; colorGroup: "Active" }

  QML.Rectangle {
      id : replyOptionsPage
      anchors.right : kmailMobile.right
      anchors.rightMargin : 70
      anchors.left : kmailMobile. left
      anchors.leftMargin : 70
      anchors.top : kmailMobile.top
      anchors.topMargin : 70
      visible : false
      color: "lightgray"
      z: 1
      //how to calculate the height needed for buttons?
      height: 195

      QML.Column {
            anchors.fill: parent
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Reply to Author" )
              onClicked : {
                application.getAction("message_reply_to_author", "").trigger();
                replyOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Reply to All" )
              onClicked : {
                application.getAction("message_reply_to_all", "").trigger();
                replyOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Reply to List" )
              onClicked : {
                application.getAction("message_reply_to_list", "").trigger();
                replyOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Discard" )
              onClicked : {
                replyOptionsPage.visible = false
              }
            }
          }
  }

  QML.Rectangle {
      id : forwardOptionsPage
      anchors.right : kmailMobile.right
      anchors.rightMargin : 70
      anchors.left : kmailMobile. left
      anchors.leftMargin : 70
      anchors.top : kmailMobile.top
      anchors.topMargin : 70
      visible : false
      color: "lightgray"
      z: 1
      //how to calculate the height needed for buttons?
      height: 145

      QML.Column {
            anchors.fill: parent
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Forward as Attachment" )
              onClicked : {
                application.getAction("message_forward_as_attachment", "").trigger();
                forwardOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Redirect" )
              onClicked : {
                application.getAction("message_redirect", "").trigger();
                forwardOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Discard" )
              onClicked : {
                forwardOptionsPage.visible = false
              }
            }
          }
  }


  QML.Rectangle {
      id : markOptionsPage
      anchors.right : kmailMobile.right
      anchors.rightMargin : 70
      anchors.left : kmailMobile. left
      anchors.leftMargin : 70
      anchors.top : kmailMobile.top
      anchors.topMargin : 70
      visible : false
      color: "lightgray"
      z: 1
      //how to calculate the height needed for buttons?
      height: 240

      QML.Column {
            anchors.fill: parent
            height : 210
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Mark as Read" )
              onClicked : {
                application.getAction("akonadi_mark_as_read", "").trigger();
                markOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Mark as Unread" )
              onClicked : {
                application.getAction("akonadi_mark_as_unread", "").trigger();
                markOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Mark as Important" )
              onClicked : {
                application.getAction("akonadi_mark_as_important", "").trigger();
                markOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Mark as Action Item" )
              onClicked : {
                application.getAction("akonadi_mark_as_action_item", "").trigger();
                markOptionsPage.visible = false
              }
            }
            KPIM.Button2 {
              width: parent.width
              buttonText : KDE.i18n( "Discard" )
              onClicked : {
                markOptionsPage.visible = false
              }
            }
          }
  }

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
      kmailActions.showOnlyCategory("mail_viewer")
   }

    onPreviousItemRequest: {
      // Only go to the previous message when currently a valid item is set.
      if ( messageView.itemId >= 0 )
        headerList.previousItem();
      kmailActions.showOnlyCategory("mail_viewer")
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
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          backToListing();
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

    QML.Image {
      id: backgroundImage
      x: 0
      y: 0
// FIXME: too big, costs about 1.5Mb RAM
//      source: "kmail-mobile-background.png"
      visible: collectionView.visible
    }

    Akonadi.AkonadiBreadcrumbNavigationView {
      id : collectionView
      anchors.top: parent.top
      width: 1/3 * parent.width
      showUnread : true
      anchors.bottom : selectButton.top
      //height : parent.height - ( collectionView.hasSelection ? 0 : selectButton.height)
      anchors.left: parent.left

      breadcrumbComponentFactory : _breadcrumbNavigationFactory

      multipleSelectionText : KDE.i18nc("%1 is e.g. 3 folders, %2 is e.g. from 2 accounts, %3 is e.g. 9 emails",
                                        "You have selected \n%1\n%2\n%3",
                                        KDE.i18np("1 folder","%1 folders",collectionView.numSelected),
                                        KDE.i18np("from 1 account","from %1 accounts",application.numSelectedAccounts),
                                        KDE.i18np("1 email","%1 emails",headerList.count))

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
      opacity : application.isHomeScreenVisible ? 1 : 0


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
    }

    QML.Rectangle {
      id : emptyFolderPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
      opacity : { (!application.isLoadingSelected && collectionView.hasSelection && headerList.count == 0 ) ? 1 : 0 }
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

     KPIM.Spinner {
      id : loadingFolderPage
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      visible : application.isLoadingSelected
    }

    QML.Rectangle {
      id : emailListPage
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
        anchors.bottom : headerList.top
        anchors.right : parent.right
        visible : false
        height : 0
      }

      HeaderView {
        id: headerList
        model: itemModel
        checkModel : _itemActionModel
        anchors.left : parent.left
        anchors.top : filterLineEdit.bottom
        anchors.bottom : parent.bottom
        anchors.right : parent.right

        showDeleteButton : true
        onItemSelected: {
          // Prevent reloading of the message, perhaps this should be done
          // in messageview itself.
          if ( messageView.itemId != headerList.currentItemId )
          {
            if (!application.isDraft(headerList.currentIndex))
            {
              messageView.messagePath = application.pathToItem(headerList.currentItemId);
              messageView.itemId = headerList.currentItemId;
              messageView.visible = true;
              backToMessageListButton.visible = true;
              collectionView.visible = false;
              emailListPage.visible = false;
              selectButton.visible = false;
              kmailActions.showOnlyCategory("mail_viewer")
            } else {
              application.restoreDraft(headerList.currentItemId);
              updateContextActionsStates()
            }
          }
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
//     visible : !favoriteSelector.visible
    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill: parent
      content: [
          KMailActions {
            id : kmailActions
            anchors.fill : parent

            scriptActions : [
              KPIM.ScriptAction {
                name : "show_about_dialog"
                script : {
                  actionPanel.collapse();
                  aboutDialog.visible = true
                }
              },
              KPIM.ScriptAction {
                name : "to_selection_screen"
                script : {
                  actionPanel.collapse();
                  favoriteSelector.visible = true;
                  mainWorkView.visible = false;
                }
              },
              KPIM.ScriptAction {
                name : "mark_as_dialog"
                script : {
                  actionPanel.collapse();
                  markOptionsPage.visible = true;
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
                  mainWorkView.visible = false
                  bulkActionScreen.visible = true
                  application.isBulkActionScreenSelected = true
                }
              },
              KPIM.ScriptAction {
                name : "attachment_save_all"
                script : {
                  actionPanel.collapse();
                  messageView.saveAllAttachments();
                }
              }
            ]

            onDoCollapse : {
              actionPanel.collapse();
            }

            onTriggered : {
              console.log("Triggered was: " + triggeredName)
            }
          }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: attachmentPanel
      visible: messageView.attachmentModel.attachmentCount >= 1 && messageView.visible
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handlePosition: actionPanel.handleHeight + actionPanel.handlePosition
      handleHeight: parent.height - actionPanel.handlePosition - actionPanel.handleHeight - anchors.topMargin - anchors.bottomMargin
      contentWidth: attachmentView.requestedWidth
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: messageView.attachmentModel
          anchors.fill: parent

          onAttachmentSelected: {
            application.openAttachment(url, mimeType);
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
    headerList : HeaderView {
      showCheckBox : true
      id: bulkActionHeaderList
      model: itemModel
      checkModel : _itemActionModel
      anchors.fill : parent
    }
    onBackClicked : {
      bulkActionScreen.visible = false
      application.isBulkActionScreenSelected = false
      mainWorkView.visible = true
    }

    KPIM.Action {
      action: application.getAction( "akonadi_mark_as_read", "" );
    }
    KPIM.Action {
      action: application.getAction( "akonadi_mark_as_important", "" );
    }
    KPIM.Action {
      action: application.getAction( "akonadi_mark_as_action_item", "" );
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
    target: messageView
    onMailRemoved : { backToListing(); }
  }

  KPIM.AboutDialog {
    id : aboutDialog
    source: backgroundImage.source
  }
}
