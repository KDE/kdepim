/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <amantia@kdab.com>

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
  id: root
  anchors.fill: parent
  color: "white"
  z: 10

  QML.MouseArea {
    anchors.fill : parent
    onClicked : {} // do nothing
  }

  QML.Rectangle {
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.left : parent. left
    anchors.leftMargin : 70
    anchors.top : parent.top
    anchors.topMargin : 70
    color: "lightgray"
    //how to calculate the height needed for buttons?
    height: 145

    QML.Column {
      anchors.fill: parent
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Forward as Attachment" )
        onClicked : {
          application.getAction("message_forward_as_attachment", "").trigger();
          root.parent.visible = false
        }
      }
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Redirect" )
        onClicked : {
          application.getAction("message_redirect", "").trigger();
          root.parent.visible = false
        }
      }
      KPIM.Button2 {
        width: parent.width
        buttonText : KDE.i18n( "Discard" )
        onClicked : {
          root.parent.visible = false
        }
      }
    }
  }
}
