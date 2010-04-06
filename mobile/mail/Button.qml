/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

import Qt 4.6

Rectangle {
  property string icon
  property string buttonText
  signal clicked
  
  radius: 12
  color: "#00000000" // Set a transparant color.

  Image {
    source: icon
    anchors.fill: parent
    anchors.margins: 5
  }

  Text {
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    text: buttonText
  }
    
  MouseArea {
    anchors.fill: parent
    hoverEnabled: true
    onEntered: { border.color = "lightblue"; border.width = 2 }
    onExited: border.width = 0
    onClicked: parent.clicked()
  }
}
