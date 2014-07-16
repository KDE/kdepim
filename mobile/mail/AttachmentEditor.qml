/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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
import org.kde.pim.mobileui 4.5 as KPIM

KPIM.ReorderListContainer {
  width: 600

  model: attachmentModel

  delegate: Item {
    id: wrapper
    width: parent.width
    height: 70
    clip: true

    Rectangle {
      id: background
      anchors.fill: parent
      opacity: (wrapper.ListView.isCurrentItem ? 0.25 : 0)
      color: "lightsteelblue"
    }
    Text {
      id: attachmentName
      anchors.fill: parent;
      text: model.attachmentName;
      horizontalAlignment: "AlignLeft";
      verticalAlignment: "AlignTop";
    }
    Row {
      id: cryptoIcons
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      Image {
        width: 22
        height: 22
        source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_signed.png" )
        visible: model.attachmentIsSigned
      }
      Image {
        width: 22
        height: 22
        source: KDE.locate( "data", "libmessageviewer/pics/mobile_status_encrypted.png" )
        visible: model.attachmentIsEncrypted
      }
    }
    Text {
      anchors.left: parent.left
      anchors.bottom: parent.bottom
      text: model.attachmentSize
    }
    MouseArea {
      anchors.fill: parent
      onClicked: wrapper.ListView.view.currentIndex = model.index
    }
  }

  onCurrentIndexChanged: attachmentEditor.setRowSelected( index )

  KPIM.ActionButton {
    icon : KDE.locate( "data", "kmail-mobile/add-attachment-button.png" )
    actionName : "attach"
  }

  KPIM.ActionButton {
    icon : KDE.locate( "data", "kmail-mobile/remove-attachment-button.png" )
    actionName : "remove"
  }

  KPIM.ActionButton {
    icon : KDE.locate( "data", "kmail-mobile/toggle-signature-button.png" )
    actionName : "toggle_attachment_signed"
  }

  KPIM.ActionButton {
    icon : KDE.locate( "data", "kmail-mobile/toggle-encryption-button.png" )
    actionName : "toggle_attachment_encrypted"
  }
}
