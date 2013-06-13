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

import QtQuick 1.1 as QML

QML.Rectangle {
  id: root
  property alias icon: iconImage.source
  property alias buttonText: buttonLabel.text
  property alias font: buttonLabel.font
  property bool enabled: true
  signal clicked

  property string _state
  height: 52
  color: "#00000000" // Set a transparent color.

  QML.BorderImage {
    id: borderImage
    anchors.fill: parent
    source: "images/button-border" + root._state + ".png"
    border { left: 14; right: 14; top: 14; bottom: 14 }
    horizontalTileMode: QML.BorderImage.Repeat
    verticalTileMode: QML.BorderImage.Stretch
  }

  QML.Image {
    id : iconImage
    anchors {
      verticalCenter : parent.verticalCenter;
      margins: (source == "" ? 0 : 5);
      left: (buttonText == "" ? undefined : parent.left);
      horizontalCenter: (buttonText == "" ? parent.horizontalCenter : undefined)
    }
    fillMode: QML.Image.PreserveAspectFit
    height: (root.height > sourceSize.height) ? sourceSize.height : (root.height - anchors.margins)
    smooth: true
  }

  QML.Text {
    id: buttonLabel
    anchors { centerIn : parent; left: iconImage.right; right: parent.right }
    color: root.enabled ? "black" : "gray"
  }

  states: [
    QML.State {
      name: "pressed"
      when : _mouseArea.pressed
      QML.PropertyChanges {
        target: root
        _state : "-active"
      }
      QML.PropertyChanges {
        target: buttonLabel
        color: "white"
      }
    }
  ]

  QML.MouseArea {
    id : _mouseArea
    anchors.fill: parent
    enabled: parent.enabled
    onClicked: parent.clicked()
  }
}
