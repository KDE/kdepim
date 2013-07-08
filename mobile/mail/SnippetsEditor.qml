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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Rectangle {
  id : _topLevel
  color : "#00000000"
  property int actionItemHeight : 70
  property int actionItemWidth : 200
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  property alias model : snippetsView.model

  property alias customActions : actionColumn.content

  signal triggered(string triggeredName)
  signal doCollapse()

  PlasmaExtras.ScrollArea {

    anchors {
      top: parent.top
      bottom: parent.bottom
      left: parent.left
    }
    width: parent.width - actionColumn.width

    flickableItem: QML.ListView {
      id: snippetsView

      model: snippetsModel
      focus: true
      clip: true

      delegate: QML.Item {
        id: snippetDelegate
        clip: true
        width: parent.width
        height: _topLevel.actionItemHeight

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
  }

  KPIM.ActionMenuContainer {
    id : actionColumn
    width : _topLevel.actionItemWidth
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    actionItemWidth : width
    actionItemHeight : _topLevel.actionItemHeight

    content : [
      KPIM.ActionListItem { name : "snippetseditor_insert_snippet"; title: KDE.i18n("Insert\nSnippet") },
      KPIM.ActionListItem { name : "snippetseditor_add_snippet"; title: KDE.i18n("Add\nSnippet") },
      KPIM.ActionListItem { name : "snippetseditor_edit_snippet"; title: KDE.i18n("Edit\nSnippet") },
      KPIM.ActionListItem { name : "snippetseditor_delete_snippet"; title: KDE.i18n("Delete\nSnippet") },
      KPIM.ActionListItem { name : "snippetseditor_add_snippetgroup"; title: KDE.i18n("Add\nGroup") },
      KPIM.ActionListItem { name : "snippetseditor_edit_snippetgroup"; title: KDE.i18n("Edit\nGroup") },
      KPIM.ActionListItem { name : "snippetseditor_delete_snippetgroup"; title: KDE.i18n("Delete\nGroup") }
    ]

    onTriggered: {
      if ( triggeredName == "snippetseditor_insert_snippet" ) {
        parent.doCollapse()
      }
    }
  }

  onActionItemSpacingChanged : {
    actionColumn.refresh();
  }

  onActionItemHeightChanged : {
    actionColumn.refresh();
  }
}
