/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

import org.kde.pim.mobileui 4.5 as KPIM

Rectangle {
  id : _topLevel
  color : "#00000000"
  property int actionItemHeight
  property int actionItemWidth
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  property alias model : myList.model
  property alias delegate : myList.delegate

  property alias customActions : actionColumn.content

  signal triggered(string triggeredName)

  Component {
    id: highlightBar
    Rectangle {
      width: myList.width
      height: 30
      color: "#FFFF88"
      y: myList.currentItem.y
    }
  }

  ListView {
    id : myList
    anchors.fill : parent
    focus: true
    highlight: highlightBar
  }

  ActionMenuContainer {
    id : actionColumn
    width : 200
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    actionItemWidth : width
    actionItemHeight : 70
  }

  onActionItemSpacingChanged : {
    myColumn.refresh();
  }

  onActionItemHeightChanged : {
    myColumn.refresh();
  }
}
