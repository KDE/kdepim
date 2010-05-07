/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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
import org.kde 4.5

Rectangle {
  property variant action

  signal triggered()

  Binding {
    target : image
    property : "pixmap"
    value : KDE.iconToPixmap(action.icon, 35)
  }

  Binding {
    target : buttonText
    property : "text"
    value : { action.text.replace("&", ""); }
  }

  visible : action.enabled
  Connections {
    target : action
    onChanged : {
      parent.visible = action.enabled
    }
  }

  radius: 12
  color: "#00000000" // Set a transparant color.

  Image {
    id : image
    anchors.verticalCenter : parent.verticalCenter
    anchors.margins: 5
  }

  Text {
    id : buttonText
    anchors.horizontalCenter : parent.horizontalCenter
    anchors.verticalCenter : parent.verticalCenter
  }

  MouseArea {
    anchors.fill : parent
    onPressed : {
      border.color = "lightblue";
      border.width = 2
    }
    onReleased : border.width = 0
    onClicked : { action.trigger(); triggered(); }
  }
}
