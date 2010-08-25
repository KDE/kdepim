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

import Qt 4.7 as QML

QML.Item {
  id : _top

  property int spaceAbove
  property int spaceBelow

  onSpaceAboveChanged : {
    lineAbove.height = spaceAbove
  }
  onSpaceBelowChanged : {
    lineBelow.height = spaceBelow
  }

  QML.Behavior on y { QML.NumberAnimation { duration : 1000; easing.type : Easing.OutQuad } }
  QML.Image {
    id : lineAbove
    anchors.bottom : active_image.top
    anchors.right : _top.right
    source : "images/dividing-line.png"
    height : 0

    QML.Behavior on height {
      QML.NumberAnimation { duration : 1000; easing.type : Easing.OutQuad }
    }
  }

  QML.Image {
    id : active_image
    anchors.right : _top.right
    anchors.rightMargin : -10
    source : "images/activeactionitem.png"
  }

  QML.Image {
    id : lineBelow
    anchors.top : active_image.bottom
    anchors.right : _top.right
    source : "images/dividing-line.png"
    height : 0
    QML.Behavior on height {
      QML.NumberAnimation { duration : 1000; easing.type : Easing.OutQuad }
    }
  }
}