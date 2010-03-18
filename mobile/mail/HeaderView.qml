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

import Qt 4.6
import org.kde 4.5

/** Akonadi Message Header List View
    @param model: The collection model to display, ETM-based, filtered to only contain a flat collection list.
    @param currentIndex: Index of the currently selected row.
 */
Rectangle {
  id: headerViewTopLevel
  property alias model: messageListView.model
  property alias currentIndex: messageListView.currentIndex
  signal messageSelected

  SystemPalette { id: palette; colorGroup: Qt.Active }
  Component {
    id: messageListDelegate

    Item {
      id: wrapper
      width: messageListView.width
      height : 68

      Rectangle {
        id: background
        opacity: 0.25
        x: 1; y: 2; width: parent.width - 2; height: parent.height - 4
        border.color: palette.mid
        radius: 5
      }
      MouseArea {
          id: pageMouse
          anchors.fill: parent
          onClicked: {
            wrapper.ListView.view.currentIndex = model.index;
            headerViewTopLevel.messageSelected()
          }
      }

      Column {
        anchors.fill: parent
        spacing: 5
        Text {
          text: model.subject
          font.bold: true
        }
        Text {
          text: "From: " + model.from
        }
        Text {
          text: "Date: " + model.date
        }
      }
    }
  }

  Component {
    id: highlight
    Rectangle {
      color: palette.highlight
      radius: 5
    }
  }

  ListView
  {
    id: messageListView
    anchors.fill: parent
    delegate : messageListDelegate
    highlight: highlight
    highlightFollowsCurrentItem: true
    focus: true
  }
}
