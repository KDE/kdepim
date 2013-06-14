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

import QtQuick 1.1

import org.kde.pim.mobileui 4.5 as KPIM

Rectangle {
  id : _topLevel
  color : "#00000000"
  property int actionItemHeight: 70
  property int actionItemWidth: 200
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  property alias model : myList.model
  property alias delegate : myList.delegate

  default property alias content : actionColumn.children

  signal triggered( string triggeredName )
  signal currentIndexChanged( int index )

  KPIM.DecoratedListView {
    id : myList
    anchors.top : _topLevel.top
    anchors.bottom : _topLevel.bottom
    anchors.left : _topLevel.left
    anchors.right : actionColumn.left
    interactive: count * actionItemHeight > height
    focus: true

    delegate : ReorderListDelegate {
        height: _topLevel.actionItemHeight
        width: ListView.view.width;
    }

    onCurrentIndexChanged : {
      _topLevel.currentIndexChanged( currentIndex )
    }
  }

  Column {
    id : actionColumn
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    width : 100
  }
}
