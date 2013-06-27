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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

PlasmaComponents.Page {

  //BEGIN: Tools
  tools: PlasmaComponents.ToolBarLayout {
    PlasmaComponents.ToolButton {

      anchors.left: parent.left

      iconSource: "go-previous"

      onClicked: pageStack.pop()
    }
  }
  //END: Tools

  KPIM.ReorderListContainer {
    id: root

    anchors.fill: parent

    actionItemHeight: parent.height * 0.2
    model: filterModel

    delegate: QML.Item {
      id: filterDelegate
      clip: true
      width: parent.width
      height: root.actionItemHeight

      QML.Rectangle {
        id: background
        anchors.fill: parent
        opacity: filterDelegate.QML.ListView.isCurrentItem ? 0.25 : 0
        color: "lightsteelblue"
      }

      QML.Text {
        anchors.fill: parent
        text: model.display
        horizontalAlignment: QML.Text.AlignHCenter
        verticalAlignment: QML.Text.AlignVCenter
      }

      QML.MouseArea {
        anchors.fill: parent
        onClicked: { filterDelegate.QML.ListView.view.currentIndex = model.index; }
      }
    }

    KPIM.ActionButton {
      icon : KDE.locate( "data", "mobileui/add-button.png" )
      actionName : "filtereditor_add"
    }

    KPIM.ActionButton {
      icon : KDE.locate( "data", "mobileui/edit-button.png" )
      actionName : "filtereditor_edit"
    }

    KPIM.ActionButton {
      icon : KDE.locate( "data", "mobileui/delete-button.png" )
      actionName : "filtereditor_delete"
    }

    KPIM.ActionButton {
      icon : KDE.locate( "data", "mobileui/moveup-button.png" )
      actionName : "filtereditor_moveup"
    }

    KPIM.ActionButton {
      icon : KDE.locate( "data", "mobileui/movedown-button.png" )
      actionName : "filtereditor_movedown"
    }

    onCurrentIndexChanged : { filterEditor.setRowSelected( index ) }
  }
}