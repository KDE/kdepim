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
  color : "#00000000"
  property int actionItemHeight: 70
  property int actionItemWidth: 200
  property int actionItemSpacing : 0
  property int bottomMargin
  anchors.bottomMargin : bottomMargin

  property alias model : myList.model
  property alias delegate : myList.delegate

  property alias upAction : upAction.name
  property alias downAction : downAction.name
  property alias deleteAction : deleteAction.name
  property alias customActions : actionColumn.content

  signal triggered(string triggeredName)

  ListView {
    id : myList
    anchors.fill : parent
    interactive: count * actionItemHeight > height
  }

  ActionMenuContainer {
    id : actionColumn
    width : _topLevel.actionItemWidth
    anchors.top : parent.top
    anchors.bottom : parent.bottom
    anchors.right : parent.right
    actionItemWidth : width
    actionItemHeight : _topLevel.actionItemHeight
    actionItemSpacing: _topLevel.actionItemSpacing
    FakeAction { id : upAction }
    FakeAction { id : downAction }
    FakeAction { id : deleteAction }
  }
}
