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

/** TODO: refactor to share common code with the other item list views. */
Item {
  id: headerViewTopLevel
  property alias model: messageListView.model
  property alias currentIndex: messageListView.currentIndex
  property alias count : messageListView.count
  property int currentMessage: -1
  signal messageSelected

  SystemPalette { id: palette; colorGroup: "Active" }

  function nextMessage() {
    if ( currentIndex < (model.itemCount - 1) ) {
      currentIndex = currentIndex + 1;
      currentMessage = model.itemId( currentIndex );
      headerViewTopLevel.messageSelected();
    }
  }

  function previousMessage() {
    if ( currentIndex > 0  ) {
      currentIndex = currentIndex - 1;
      currentMessage = model.itemId( currentIndex );
      headerViewTopLevel.messageSelected();
    }
  }

  Component {
    id: messageListDelegate

    Item {
      id: wrapper
      width: messageListView.width
      height: 72
      clip: true

      Rectangle {
        id: background
        x: 1; y: 2; width: parent.width - 2; height: parent.height - 4
        border.color: palette.mid
        opacity: 0.25
        radius: 5
      }
      MouseArea {
        anchors.fill: parent
        onClicked: {
          var nonCurrentClicked = false
          if ( headerViewTopLevel.currentIndex == model.index ) { nonCurrentClicked = true }
          wrapper.ListView.view.currentIndex = model.index
          headerViewTopLevel.currentMessage = model.itemId
          if ( nonCurrentClicked ) { headerViewTopLevel.messageSelected() }
        }
      }

      Row {
        anchors.fill: background
        Image {
          pixmap: model.picture
        }
        Text {
          text: model.name
        }
      }

    }
  }

  ListView
  {
    id: messageListView
    anchors.fill: parent
    delegate : messageListDelegate
    highlightFollowsCurrentItem: true
    highlightRangeMode: "StrictlyEnforceRange"
    preferredHighlightBegin: height/2 - currentItem.height/2
    preferredHighlightEnd: height/2 + currentItem.height/2
    focus: true
    clip: true
  }
}
