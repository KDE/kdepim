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

Item {
  id : _topContext

  Rectangle {
    id : favoritesList
    color : "blue"
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : parent.left
    width : parent.width / 3
  }

  Rectangle {
    id : accountsList
    color : "red"
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : favoritesList.right
    width : parent.width / 3
  }

  Rectangle {
    id : actionsList
    color : "yellow"
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.left : accountsList.right
    width : parent.width / 3

    Text {
      id: actionLabel
      text: "Actions"
      style: Text.Sunken
      anchors.horizontalCenter: parent.horizontalCenter
    }
    Button {
      id: newMailButton
      anchors.top: actionLabel.bottom;
      anchors.horizontalCenter: parent.horizontalCenter;
      width: parent.width - 10
      height: parent.height / 6
      buttonText: "New Mail"
    }
    Button {
      id: newAccountButton
      anchors.top: newMailButton.bottom;
      anchors.horizontalCenter: parent.horizontalCenter;
      width: parent.width - 10
      height: parent.height / 6
      buttonText: "New Account"
      onClicked : { application.launchAccountWizard(); }
    }
  }
}
