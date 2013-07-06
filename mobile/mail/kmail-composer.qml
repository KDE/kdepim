/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

import QtQuick 1.1
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.messagecomposer 4.5 as MessageComposer
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

KPIM.MainView {
  id: root

  PlasmaExtras.ScrollArea {
    id: flick
    anchors.fill: parent

    flickableItem: Flickable {

      contentHeight: editorView.contentHeight;

      contentItem.children: [
      EditorView {
        id: editorView
        enabled: !window.busy
        anchors.fill: parent
        screenHeight: root.height
      }
      ]
    }
  }

  SlideoutPanelContainer {
    enabled: !window.busy
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: folderPanel
      titleText: KDE.i18n( "Recipients" )
      handleHeight: 150
      handlePosition: 40
      content: [
      PlasmaExtras.ScrollArea {

        id: flickablerecipients
        anchors.fill: parent

        flickableItem: Flickable {
          contentHeight: recipientsEditor.height;

          contentItem.children: [
          Item {
            id: recipientswrapper
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            MessageComposer.RecipientsEditor {
              id: recipientsEditor
              anchors.top: parent.top
              anchors.left: parent.left
              anchors.right: parent.right
            }
          }
          ]
        }
      }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handleHeight: 150
      handlePosition: folderPanel.handlePosition + folderPanel.handleHeight

      Component.onCompleted: {
        kmailComposerActions.showOnlyCategory( "composer" )
        actionPanel.expanded.connect( kmailComposerActions, kmailComposerActions.refresh );
      }

      content: [
          KMailComposerActions {
            id : kmailComposerActions
            anchors.fill : parent

            scriptActions : [

            KPIM.ScriptAction {
              name: "composer_configure_identity"
              script: {
                actionPanel.collapse();
                window.configureIdentity();
              }
            },
            KPIM.ScriptAction {
              name: "composer_configure_transport"
              script: {
                actionPanel.collapse();
                window.configureTransport();
              }
            },
            KPIM.ScriptAction {
              name: "composer_close"
              script: {
                actionPanel.collapse();
                window.close();
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
      handleHeight: 100
      handlePosition: actionPanel.handlePosition + actionPanel.handleHeight
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      contentWidth: attachmentEditorView.width
      content: [
        AttachmentEditor {
          id: attachmentEditorView
          anchors.fill: parent
        }
      ]
    }
  }

  // ### Make it a general processing screen?
  Rectangle {
      id: busyView
      visible: window.busy;
      z: 99

      PlasmaComponents.BusyIndicator {
          anchors.centerIn: parent
          running: true
      }

      color: "grey"
      opacity: 0.5
      anchors.fill: parent
  }
}
