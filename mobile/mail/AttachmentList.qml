/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

/**
 * Shows a list view of the specified attachment model of a MessageView component.
 * @param model An attachment model
 */
Item {
  property alias model: attachmentListView.model

  Component {
    id: attachmentDelegate
    Item {
      id: wrapper
      width: attachmentListView.width
      height: 48
      clip: true

      Rectangle {
        id: background
        anchors.fill: parent
        opacity: (isCurrentItem ? 0.25 : 0)
      }
      Text {
        anchors.fill: parent;
        text: model.display;
        horizontalAlignment: "AlignHCenter";
        verticalAlignment: "AlignVCenter";
      }

      MouseArea {
        anchors.fill: parent
        onClicked: {
          console.log( "TODO - attachment selected: " + model.display );
        }
      }
    }
  }

   ListView {
    id: attachmentListView
    anchors.fill: parent

    delegate: attachmentDelegate

    MouseArea {
      anchors.fill: parent
      onClicked: {
        console.log( "current attachment count: " + model.attachmentCount );
      }
    }
  }

}
