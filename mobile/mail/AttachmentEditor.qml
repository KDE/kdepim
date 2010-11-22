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

import Qt 4.7
import org.kde.pim.mobileui 4.5 as KPIM

Item {
  width: 600

  ListView {
    id: attachmentListView
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    width: parent.width / 2
    model: attachmentModel
    clip: true

    delegate: Item {
      id: wrapper
      width: parent.width
      height: 70
      clip: true

      Rectangle {
        id: background
        anchors.fill: parent
        opacity: (wrapper.ListView.isCurrentItem ? 0.25 : 0)
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
        onClicked: {
          wrapper.ListView.view.currentIndex = model.index
        }
      }
    }

    onCurrentIndexChanged : {
      attachmentEditor.setRowSelected( currentIndex )
    }
  }

  KPIM.ActionMenuContainer {
    id : actionColumn
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    actionItemWidth : width
    actionItemHeight : 30
    width: parent.width/2

    content : [
      KPIM.ActionListItem { name : "attach" },
      KPIM.ActionListItem { name : "remove" },
      KPIM.ActionListItem { name : "toggle_attachment_signed" },
      KPIM.ActionListItem { name : "toggle_attachment_encrypted" }
    ]
  }
}
