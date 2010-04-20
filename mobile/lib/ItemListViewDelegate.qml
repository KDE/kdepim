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

import Qt 4.7

Item {
  id: itemViewTopLevel
  property alias summaryContent: itemSummary.data
  property alias summaryContentHeight: itemSummary.height
  property alias detailsContent: itemDetailsContent.data
  property alias detailsContentHeight: itemDetailsContent.height

  width: itemListView.width
  height: 32
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
//      var nonCurrentClicked = false
//      if ( itemViewTopLevel.currentIndex == model.index ) { nonCurrentClicked = true }
      itemViewTopLevel.ListView.view.currentIndex = model.index
//      itemViewTopLevel.ListView.view.currentItemId = model.itemId
//      currentIdemIdChanged( model.itemId )
//      if ( nonCurrentClicked ) { itemViewTopLevel.itemSelected() }
    }
  }

  Column {
    anchors.fill: background
    anchors.top: background.top
    anchors.left: background.left
    anchors.leftMargin: 5
    anchors.topMargin: 5
    spacing: 10

    Item {
      id: itemSummary
      height: 12
    }

    Item {
      id: itemDetails
      anchors.top: itemSummary.bottom
      opacity: 0
      Item {
        id: itemDetailsContent
      }
    }
  }

  states: [
    State {
      name: "currentState"
      when: itemViewTopLevel.ListView.isCurrentItem
      PropertyChanges { target: itemViewTopLevel; height: 100 }
      PropertyChanges { target: itemDetails; opacity: 1 }
      PropertyChanges { target: background; color: palette.highlight; opacity: 1.0 }
    }
  ]
  transitions: [
    Transition {
      NumberAnimation { property: "height"; duration: 200 }
      NumberAnimation { target: itemDetails; property: "opacity"; duration: 200 }
      ColorAnimation { target: itemDetails; property: "color"; duration: 200 }
      ColorAnimation { target: background; property: "color"; duration: 200 }
    }
  ]
}
