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

/** Delegate base class for use in ItemListView */
Item {
  id: itemViewTopLevel
  /// content in summary mode
  property alias summaryContent: itemSummary.data
  /// height of an item in summary mode
  property int summaryContentHeight: 32
  /// content in details mode
  property alias detailsContent: itemDetails.data
  /// height of an item in detail mode
  property int detailsContentHeight: 100

  width: itemListView.width
  height: summaryContentHeight
  clip: true

  SystemPalette { id: palette; colorGroup: "Active" }

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
      itemViewTopLevel.ListView.view.currentIndex = model.index;
      itemViewTopLevel.ListView.view.parent.currentItemId = model.itemId;

      itemViewTopLevel.ListView.view.parent.itemSelected();
      application.setListSelectedRow(model.index);
    }
  }

  Item {
    anchors.fill: background
    anchors.margins: 4

    Item {
      id: itemSummary
      anchors.fill: parent
    }

    Item {
      id: itemDetails
      anchors.fill: parent
      opacity: 0
    }
  }
}
