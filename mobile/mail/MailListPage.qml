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
  property variant checkModel: _itemActionModel

  implicitWidth: pageRow.width * 2 /3

  //BEGIN Tools
  tools: PlasmaComponents.ToolBarLayout {

    PlasmaComponents.ToolButton {
      iconSource: "go-previous"

      onClicked: pageRow.pop()
    }

    //TODO (de)select-all checkbox
    Row {

      anchors.horizontalCenter: parent.horizontalCenter

      opacity: checkModel.hasSelection ? 1 : 0.5
      spacing: root.width * 0.03

      PlasmaComponents.ToolButton {
        iconSource: "mail-mark-unread"

        onClicked: application.getAction("akonadi_mark_as_read", "").trigger()
      }

      PlasmaComponents.ToolButton {
        iconSource: "mail-mark-important"

        onClicked: application.getAction("akonadi_mark_as_important", "").trigger()
      }

      //TODO usability feature: offer to undo deletion
      PlasmaComponents.ToolButton {
        iconSource: "edit-delete"

        onClicked: application.getAction("akonadi_move_to_trash", "").trigger()
      }
    }


    //TODO add new mail from template once the multiple actions button is ready
    PlasmaComponents.ToolButton {

      anchors.right: parent.right

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
    currentIndex: -1


    onCurrentRowChanged: {
      if (navigationModel != undefined)
        navigationModel.select(currentRow, 3)
    }

    Connections {
      target : navigationModel
      onCurrentRowChanged: currentRow = navigationModel.currentRow
    }

    delegate: PlasmaComponents.ListItem {
      id: headerListDelegate

      height: label.height * 2.5

      clip: true
      enabled: true
      checked: threadView.currentIndex == index

      onClicked: {
        pageRow.pop(root)
        threadView.currentIndex = index
        pageRow.push(Qt.createComponent("MailViewPage.qml"))
        navigationModel.select(model.index, 3)
      }

      onPressAndHold: threadView.currentIndex = index

      Rectangle {
        id: itemBackground

        anchors.fill: parent
        color: checked == true ? "lightgrey" : "white"
        opacity: 0.5
      }

      PlasmaComponents.CheckBox {
        id: checkBox

        anchors {
          left: parent.left
          leftMargin: label.width
          verticalCenter: parent.verticalCenter
        }

        checked: model.checkOn

        onClicked: checkModel.select(model.index, 8)
      }

      PlasmaComponents.Label {
        id: fromLabel

        anchors {
          top : parent.top
          left : checkBox.right
          leftMargin: label.width
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
          right: statusIcon.left
        }

        text: model.date
        horizontalAlignment: "AlignRight"
        font.weight: Font.Light
        color : "#0C55BB"
      }

      PlasmaExtras.Heading {
        id: subjectLabel

        anchors {
          bottom: parent.bottom
          left: checkBox.right
          leftMargin: label.width
          right: statusIcon.left
        }

        level: 4
        text: model.subject
        elide: "ElideRight"
        color: model.is_unread ? "#E10909" : "#3B3B3B"
      }

      PlasmaComponents.ToolButton {
        id: statusIcon

        anchors{
          right: parent.right
          verticalCenter: parent.verticalCenter
        }

        height: parent.height * 0.8

        iconSource: model.is_important ? "mail-mark-important" : "mail-mark-unread"

        onClicked: dialog.open()

        //BEGIN Dialog
        PlasmaComponents.Dialog {
          id: dialog

          visualParent: parent

          buttons: Column {

            spacing: root.width * 0.03

            PlasmaComponents.ToolButton {

              iconSource: "mail-mark-unread"

              onClicked: {
                checkModel.select(model.index, 3)
                application.getAction("akonadi_mark_as_read", "").trigger()
                checkModel.select(-1, 1)
              }

            }

            PlasmaComponents.ToolButton {

              iconSource: "mail-mark-important"

              onClicked: {
                checkModel.select(model.index, 3)
                application.getAction("akonadi_mark_as_important", "").trigger()
                checkModel.select(-1, 1)
              }

            }

            PlasmaComponents.ToolButton {

              iconSource: "edit-delete"

              onClicked: {
                checkModel.select(model.index, 3)
                application.getAction("akonadi_move_to_trash", "").trigger()
                checkModel.select(-1, 1)
              }

            }
          }
        }
        //END Dialog
      }
    }
  }

  PlasmaComponents.Label {
    id: label

    text: "   "
  }
}