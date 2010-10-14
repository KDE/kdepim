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

  QML.Connections {
    target: guiStateManager
    onGuiStateChanged: { updateContextActionStates() }
  }

  QML.Component.onCompleted : updateContextActionStates();

  function updateContextActionStates()
  {
    if ( guiStateManager.inHomeScreenState ) {
      kmailActions.showOnlyCategory( "home" )
    } else if ( guiStateManager.inAccountScreenState ) {
      kmailActions.showOnlyCategory( "account" )
    } else if ( guiStateManager.inSingleFolderScreenState ) {
      kmailActions.showOnlyCategory( "single_folder" )
    } else if ( guiStateManager.inMultipleFolderScreenState ) {
      kmailActions.showOnlyCategory( "multiple_folder" )
    } else if ( guiStateManager.inViewSingleItemState ) {
      kmailActions.showOnlyCategory( "mail_viewer" )
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
    visible : guiStateManager.inViewSingleItemState
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

  QML.Rectangle {
    id : backToMessageListButton
    visible : guiStateManager.inViewSingleItemState
    anchors.right : kmailMobile.right
    anchors.rightMargin : 70
    anchors.bottom : kmailMobile.bottom
    anchors.bottomMargin : 100
    QML.Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          guiStateManager.popState();
        }
      }
    }
  }

  QML.Item {
    id : mainWorkView
    visible: { guiStateManager.inHomeScreenState ||
               guiStateManager.inAccountScreenState ||
               guiStateManager.inSingleFolderScreenState ||
               guiStateManager.inMultipleFolderScreenState
             }
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

      onSelectedClicked : {
        guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState )
      }

      KPIM.AgentStatusIndicator {
        anchors { top: parent.top; right: parent.right; rightMargin: 10; topMargin: 10 }
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
        guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState )
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
      id : emptyFolderPage
      visible: (!application.isLoadingSelected && !guiStateManager.inHomeScreenState && collectionView.hasBreadcrumbs && headerList.count == 0)
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"
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
      visible: { guiStateManager.inAccountScreenState ||
                 guiStateManager.inSingleFolderScreenState ||
                 guiStateManager.inMultipleFolderScreenState
               }
      anchors.left : collectionView.right
      anchors.top : parent.top
      anchors.bottom : parent.bottom
      anchors.right : parent.right
      color : "#00000000"

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

        showDeleteButton : false // too easy to accidentally hit it, although very useful...
        onItemSelected: {
          // Prevent reloading of the message, perhaps this should be done
          // in messageview itself.
          if ( messageView.itemId != headerList.currentItemId )
          {
            if (!application.isDraft(headerList.currentIndex))
            {
              messageView.messagePath = application.pathToItem(headerList.currentItemId);
              messageView.itemId = headerList.currentItemId;
              guiStateManager.pushUniqueState( KPIM.GuiStateManager.ViewSingleItemState );
            } else {
              application.restoreDraft(headerList.currentItemId);
              updateContextActionStates()
            }
          }
        }
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent
    visible: !guiStateManager.inBulkActionScreenState && !guiStateManager.inMultipleFolderSelectionScreenState
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
                name : "configure"
                script : {
                  actionPanel.collapse();
                  configDialog.visible = true;
                }
              },
              KPIM.ScriptAction {
                name : "to_selection_screen"
                script : {
                  actionPanel.collapse();
                  guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
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
                  guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
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
      property bool showActions: attachmentView.showActions
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: messageView.attachmentModel
          anchors.fill: parent

          onOpenAttachment: {
            application.openAttachment(url, mimeType);
          }

          onSaveAttachment: {
            application.saveAttachment(url);
          }
        }
      ]
    }
  }

  KPIM.MultipleSelectionScreen {
    id : favoriteSelector
    visible : guiStateManager.inMultipleFolderSelectionScreenState
    anchors.fill : parent
    backgroundImage : backgroundImage.source
    onFinished : {
      guiStateManager.popState();
      application.clearPersistedSelection("preFavSelection");
      application.multipleSelectionFinished();
    }
    onCanceled : {
      guiStateManager.popState();
      application.restorePersistedSelection("preFavSelection");
    }
  }

  KPIM.BulkActionScreen {
    id : bulkActionScreen
    visible : guiStateManager.inBulkActionScreenState
    anchors.top: parent.top
    anchors.topMargin : 12
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right : parent.right
    backgroundImage : backgroundImage.source

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
      guiStateManager.popState();
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

  ConfigDialog {
    id: configDialog
    visible: false
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
    onMailRemoved : { guiStateManager.popState(); }
  }

  KPIM.AboutDialog {
    id : aboutDialog
    source: backgroundImage.source
  }
}
