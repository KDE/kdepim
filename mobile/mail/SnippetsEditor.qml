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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

QML.Rectangle {
  anchors.fill: parent
  z: 10
  color: "#00000000"

  QML.ListView {
    id: snippetsView
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.leftMargin: 30
    anchors.topMargin: 10
    width: parent.width * 0.6
    model: snippetsModel
    clip: true

    delegate: QML.Item {
      id: snippetDelegate
      clip: true
      width: parent.width
      height: 70

      QML.Rectangle {
        id: background
        anchors.fill: parent
        opacity: snippetDelegate.QML.ListView.isCurrentItem ? 0.25 : 0
        color: "lightsteelblue"
      }

      QML.Text {
        anchors.fill: parent
        anchors.leftMargin: model.isSnippetGroup ? 5 : 25
        text: model.display
        font.bold: model.isSnippetGroup
        horizontalAlignment: QML.Text.AlignLeft
        verticalAlignment: QML.Text.AlignVCenter
      }

      QML.MouseArea {
        anchors.fill: parent
        onClicked: { snippetDelegate.QML.ListView.view.currentIndex = model.index; }
      }
    }

    onCurrentIndexChanged : {
      snippetsEditor.setRowSelected( currentIndex )
    }
  }

  KPIM.ActionMenuContainer {
    id : actionColumn
    anchors.left: snippetsView.right
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 10
    anchors.bottom: parent.bottom
    actionItemWidth: width
    actionItemHeight: 50

    content : [
      KPIM.ActionListItem { name : "snippetseditor_insert_snippet" },
      KPIM.ActionListItem { name : "snippetseditor_add_snippet" },
      KPIM.ActionListItem { name : "snippetseditor_edit_snippet" },
      KPIM.ActionListItem { name : "snippetseditor_delete_snippet" },
      KPIM.ActionListItem { name : "snippetseditor_add_snippetgroup" },
      KPIM.ActionListItem { name : "snippetseditor_edit_snippetgroup" },
      KPIM.ActionListItem { name : "snippetseditor_delete_snippetgroup" }
    ]
  }
}
