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

import Qt 4.7

/** Delegate base class for use in ItemListView */
Item {
  id: itemViewTopLevel
  property alias summaryContent: itemSummary.data
  property alias showCheckBox : checkBoxImage.visible
  property variant checkModel

  width: itemListView.width

  SystemPalette { id: palette; colorGroup: "Active" }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      if (showCheckBox && checkModel) {
        checkModel.select(model.index, 8)
      } else {
        itemViewTopLevel.ListView.view.currentIndex = model.index;
        itemViewTopLevel.ListView.view.parent.currentItemId = model.itemId;

        itemViewTopLevel.ListView.view.parent.itemSelected();
        // We also need to check it in the action check model so
        // that actions related to it get enabled.
        //checkModel.select(model.index, 3)
        application.setListSelectedRow(model.index);
      }
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
  Image {
    id : checkBoxImage
    anchors.verticalCenter: parent.verticalCenter;
    anchors.right : parent.right;
    visible : false
    opacity : model.checkOn ? 1 : 0
    source : "images/check.png"
  }

  Connections {
    target : application
    onSelectedItemChanged : {
      itemViewTopLevel.ListView.view.currentIndex = row;
      itemViewTopLevel.ListView.view.parent.currentItemId = itemId;

      itemViewTopLevel.ListView.view.parent.itemSelected();
    }
  }
}
