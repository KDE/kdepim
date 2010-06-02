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

Rectangle {
  property string icon
  property alias buttonText: buttonText.text
  property alias font: buttonText.font
  signal clicked

  color : "#00000000"
  height : rightPart.height

  Image {
    id : leftPart
    source : "button-left.png"
    anchors.left : parent.left
    anchors.top : parent.top
//     anchors.bottom : parent.bottom
  }
  Image {
    source : "button-center.png"
    anchors.left : leftPart.right
    anchors.right : rightPart.left
    fillMode : Image.TileHorizontally
    anchors.top : parent.top
//     anchors.bottom : parent.bottom
  }
  Image {
    id : rightPart
    source : "button-right.png"
    anchors.right : parent.right
    anchors.top : parent.top
//     anchors.bottom : parent.bottom
  }

  Text {
    id: buttonText
    verticalAlignment : Text.AlignVCenter
    horizontalAlignment : Text.AlignHCenter
    anchors.fill : parent
//     anchors.horizontalCenter: parent.horizontalCenter
//     anchors.verticalCenter: parent.verticalCenter
  }

  MouseArea {
    anchors.fill: parent
    onPressed: {
      // Change images?
    }
    onReleased: {
      // Change images?
    }
    onClicked: parent.clicked()
  }
}
