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

import QtQuick 1.1 as QML
import org.kde.pim.mobileui 4.5 as KPIM


QML.Item {
  id : snippetsList_top
  width : parent.width

  property string category
  property string name

  property alias text : nameItem.buttonText
  property alias totalWidth : _listContent.width
  property alias columnHeight : _listContent.height
  property alias depth : _listContent.bottomMargin
  property alias actionItemHeight: _listContent.actionItemHeight

  property alias showChildren : _listContent.visible
  signal triggered(string triggeredName)
  signal doCollapse()

  onVisibleChanged :
  {
    if (!visible)
      height = -actionItemSpacing
    else
      height = actionItemHeight
  }


  KPIM.Button {
    id : nameItem
    height : parent.height
    width : parent.width

    onClicked : {
      snippetsList_top.triggered(snippetsList_top.name)
      showChildren = true
    }
  }

  SnippetsEditor {
    id : _listContent
    anchors.left : nameItem.right
    anchors.bottom : nameItem.top

    property int bottomMargin
    anchors.bottomMargin : bottomMargin
    visible : false

    onTriggered : {
      snippetsList_top.triggered(triggeredName)
    }

    onDoCollapse : {
      snippetsList_top.doCollapse();
    }
  }
}
