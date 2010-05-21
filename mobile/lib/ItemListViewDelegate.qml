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
  property alias summaryContent: itemSummary.data

  width: itemListView.width
  clip: true

  SystemPalette { id: palette; colorGroup: "Active" }

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
    anchors.fill: parent
    anchors.margins: 4
    id: itemSummary
  }
  Rectangle {
    id: bottomLine
    x: 1; y: parent.height -2; width: parent.width - 2; height: 1
    border.color: palette.mid
    opacity: 0.25
  }
}
