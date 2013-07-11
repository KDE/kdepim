/*
 *  Copyright 2013 (C) Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaComponents.Page {
  id: root

  property variant navigationModel: _threadSelector

  implicitWidth: pageRow.width * 2 /3

  //BEGIN Tools
  tools: PlasmaComponents.ToolBarLayout {

    PlasmaComponents.ToolButton {
      iconSource: "go-previous"

      onClicked: pageRow.pop()
    }

    PlasmaComponents.ToolButton {
      iconSource: "mail-message-new"

      onClicked: application.startComposer()
    }
  }
  //END Tools

  ListView {
    id : threadView

    anchors.fill: parent

    property int currentItemId: -1
    property int currentRow : -1

    model: _threads

    focus: true
    clip: true

    onCurrentRowChanged: {
      if (navigationModel != undefined)
        navigationModel.select(currentRow, 3)
    }

    Connections {
      target : navigationModel
      onCurrentRowChanged : {
        currentRow = navigationModel.currentRow
      }
    }

    delegate: PlasmaComponents.ListItem {
      id: headerListDelegate

      height: root.height / 8
      clip: true

      MouseArea {
        anchors.fill: parent
        onClicked: {
          pageRow.pop(root)
          pageRow.push(Qt.createComponent("MailViewPage.qml"))
          navigationModel.select(model.index, 3)
        }
      }

      Rectangle {
        id: itemBackground

        anchors.fill: parent
      }

      PlasmaComponents.Label {
        id: fromLabel

        anchors {
          top : parent.top
          left : parent.left
          right: dateLabel.left
        }

        text : model.from
        elide: "ElideRight"
        font.weight: Font.Light
        color : "#0C55BB"
      }

      PlasmaComponents.Label {
        id: dateLabel

        anchors {
          top: parent.top
          right: parent.right
        }

        text: model.date
        horizontalAlignment: "AlignRight"
        font.weight: Font.Light
        color : "#0C55BB"
      }

      PlasmaExtras.Heading {
        id: subjectLabel

        anchors {
          top: fromLabel.bottom
          left: parent.left
          right: parent.right
        }

        level: 4
        text: model.subject
        elide: "ElideRight"
        color: model.is_unread ? "#E10909" : "#3B3B3B"
      }
    }
  }
}

