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

  function load()
  {
    aclEditor.load()
  }

  QML.Text {
    id: headLine
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.leftMargin: 30
    anchors.topMargin: 40

    text: KDE.i18n( "<b>Access Control List for '%1'</b>", aclEditor.collectionName )
  }

  QML.ListView {
    id: aclView
    anchors.left: parent.left
    anchors.top: headLine.bottom
    anchors.bottom: okButton.top
    anchors.leftMargin: 30
    anchors.topMargin: 10
    width: parent.width * 0.6
    model: aclModel
    clip: true

    delegate: KPIM.ReorderListDelegate {
      height: 20
      width: parent.width
    }

    onCurrentIndexChanged : {
      aclEditor.setRowSelected( currentIndex )
    }
  }

  KPIM.ActionMenuContainer {
    id : actionColumn
    anchors.left: aclView.right
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.topMargin: 50
    actionItemWidth: width
    actionItemHeight: 70

    content : [
      KPIM.ActionListItem { name : "acleditor_add" },
      KPIM.ActionListItem { name : "acleditor_edit" },
      KPIM.ActionListItem { name : "acleditor_delete" }
    ]
  }

  KPIM.Button2 {
    id: okButton
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.leftMargin: 30
    width: 150
    buttonText: KDE.i18n( "Save" )
    onClicked: {
      aclEditor.save();
      guiStateManager.popState();
    }
  }

  KPIM.Button2 {
    id: cancelButton
    anchors.left: okButton.right
    anchors.bottom: parent.bottom
    anchors.leftMargin: 10
    width: 150
    buttonText: KDE.i18n( "Cancel" )
    onClicked: {
      guiStateManager.popState();
    }
  }
}
