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
import org.kde.messageviewer 4.5 as MessageViewer
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Page {
  id: root

  //BEGIN Tools
  tools: PlasmaComponents.ToolBarLayout{
    PlasmaComponents.ToolButton{
      iconSource: "go-previous"

      onClicked: pageStack.pop()
    }

    Row {

      spacing: root.width * 0.03

      PlasmaComponents.ToolButton {
        iconSource: "mail-forward"

        onClicked: application.getAction("message_forward", "").trigger()
      }

      PlasmaComponents.ToolButton {
        iconSource: "mail-reply-sender"

        onClicked: application.getAction("message_reply_to_author", "").trigger()
      }

      PlasmaComponents.ToolButton {
        iconSource: "mail-reply-all"

        onClicked: application.getAction("message_reply_to_all", "").trigger()
      }
    }
  }
  //END Tools

  //BEGIN MessageView
  MessageViewer.MessageView {
    id: messageView

    anchors {
      fill: parent
      leftMargin: parent.width * 0.05
      rightMargin: parent.width * 0.05
    }

    clip: true
  }
  //END MessageView

}
