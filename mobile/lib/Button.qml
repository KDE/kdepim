/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

import QtQuick 1.1

Rectangle {
  property string icon
  property alias buttonText: buttonText.text
  property alias font: buttonText.font
  signal clicked

  radius: 12
  color: "#00000000" // Set a transparent color.

  Image {
    source: icon
    anchors.fill: parent
    anchors.margins: 5
  }

  Text {
    id: buttonText
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
  }

  MouseArea {
    anchors.fill: parent
    onPressed: {
      border.color = "#4166F5";
      border.width = 2
    }
    onReleased: border.width = 0
    onCanceled : border.width = 0
    onClicked: parent.clicked()
  }
}
