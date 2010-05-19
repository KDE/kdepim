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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.messagecomposer 4.5 as MessageComposer

KPIM.MainView {

  EditorView {
    id: editorView
    anchors.fill: parent
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      anchors.fill: parent
      id: folderPanel
      titleText: KDE.i18n( "Recipients" )
      handleHeight: 150
      content: [
        MessageComposer.RecipientsEditor {
          id: recipientsEditor
          anchors.fill: parent
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
          id: configureTransportButton
          anchors.top: parent.top;
          anchors.horizontalCenter: parent.horizontalCenter;
          width: parent.width - 10
          height: parent.height / 6
          buttonText: KDE.i18n( "Configure Transport" )
          onClicked: {
            actionPanel.collapse();
            window.configureTransport();
          }
        }
      ]
    }

    SlideoutPanel {
      anchors.fill: parent
      id: attachmentPanel
      titleIcon: KDE.iconPath( "mail-attachment", 48 );
      handleHeight: 30
      contentWidth: attachmentView.requestedWidth
      content: [
        KPIM.AttachmentList {
          id: attachmentView
          model: attachmentModel
          anchors.top: parent.top
          anchors.left : parent.left
          anchors.right : parent.right
          height : ( parent.height / 6 ) * 5
        },
        KPIM.Action {
          id : newAttachmentButton
//           color : "lightsteelblue"
          anchors.top : attachmentView.top
          anchors.left : parent.left
          anchors.right : parent.right
          height : parent.height / 6
          action : application.getAction("add_attachment")
          onTriggered : {
            console.log("New Attachment");
          }
        }
      ]
    }
  }

}
