/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <amantia@kdab.com>
    Copyright (C) 2013 Michael Bohlender <michael.bohlender@kdemail.net>

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
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Page {

  anchors.fill: parent

  tools: PlasmaComponents.ToolBarLayout {

    PlasmaComponents.ToolButton {

      anchors.left: parent.left

      iconSource: "go-previous"

      onClicked: pageRow.pop()
    }
  }

  QML.Rectangle {
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.left : parent.left
    anchors.leftMargin : 70
    anchors.top : parent.top
    anchors.topMargin : 70
    color: "lightgray"
    //how to calculate the height needed for buttons?
    height: 195

    QML.Column {
      anchors.fill: parent
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Reply to Author" )
        onClicked : {
          application.getAction("message_reply_to_author", "").trigger();
          pageRow.pop()
        }
      }
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Reply to All" )
        onClicked : {
          application.getAction("message_reply_to_all", "").trigger();
          pageRow.pop()
        }
      }
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Reply to List" )
        onClicked : {
          application.getAction("message_reply_to_list", "").trigger();
          pageRow.pop()
        }
      }
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Reply Without Quoting" )
        onClicked : {
          application.getAction("message_reply_without_quoting", "").trigger();
          pageRow.pop()
        }
      }
    }
  }
}
