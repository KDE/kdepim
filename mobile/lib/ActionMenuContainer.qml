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

import org.kde.pim.mobileui 4.5 as KPIM

Rectangle {
  id : _topLevel
  color : "lightsteelblue"
  property int actionItemHeight
  property int actionItemWidth
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  default property alias content : myColumn.children

  signal triggered(string triggeredName)

  Column {
    height : parent.height
    width : parent.actionItemWidth
    id : myColumn
    spacing : actionItemSpacing

    function triggered(triggeredName) {
      _topLevel.triggered(triggeredName)

      for ( var i = 0; i < children.length; ++i ) {
        if (children[i].name != triggeredName)
          children[i].showChildren = false;
      }

    }

    function refresh() {
      for ( var i = 0; i < children.length; ++i ) {
        children[i].height = actionItemHeight
        if (children[i].columnHeight != undefined)
          children[i].columnHeight = myColumn.height
        if (children[i].totalWidth != undefined)
          children[i].totalWidth = _topLevel.width - actionItemWidth
        if (children[i].depth != undefined)
          children[i].depth = i * ( actionItemHeight + actionItemSpacing )
        if (children[i].actionItemSpacing != undefined)
          children[i].actionItemSpacing = actionItemSpacing
        if (children[i].actionItemHeight != undefined)
          children[i].actionItemHeight = actionItemHeight
        children[i].triggered.disconnect( this, triggered )
        children[i].triggered.connect( this, triggered )
        // children[i].width = parent.actionItemWidth
      }
    }

    onChildrenChanged : {
      refresh();
    }
  }

  onActionItemSpacingChanged : {
    myColumn.refresh();
  }

  onActionItemHeightChanged : {
    myColumn.refresh();
  }
}