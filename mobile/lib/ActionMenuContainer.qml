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

Item {
  id : _topLevel
  property int actionItemHeight
  property int actionItemWidth
  property int actionItemSpacing : 0
  property int bottomMargin
  property bool menuStyle : false
  property int activeCount: 0
  anchors.bottomMargin : bottomMargin
  property string cachedCategory

  property alias scriptActions : myScriptActions.data

  default property alias content : itemModel.children

  signal triggered(string triggeredName)

  signal doCollapse()

  function refresh()
  {
    showOnlyCategory( cachedCategory )
    selectTopItem()
  }

  function selectTopItem()
  {
    for ( var i = 0; i < itemModel.children.length; ++i ) {
      if ( itemModel.children[i].visible ) {
        if ( itemModel.children[i].preselectAction )
          itemModel.children[i].preselectAction();
        return;
      }
    }
  }

  clip: true

  Item {
    id : myScriptActions

    function trigger(name)
    {
      for (var i = 0; i < children.length; ++i )
      {
        if (children[i].name == name)
        {
          children[i].trigger();
          return;
        }
      }
    }
  }

  function showOnlyCategory(category)
  {
    cachedCategory = category
    itemModel.showOnlyCategory(category)

    var firstMenu = -1;
    var showFirstMenu = true;
    for ( var i = 0; i < itemModel.children.length; ++i ) {
      // QML has no type or interface system so we rely on the all children of itemModel
      // that are items have a category property
      if (firstMenu == -1 && itemModel.children[i].visible && itemModel.children[i].showChildren != undefined ) {
        // We make the first visible item show its children.
        firstMenu = i;
      }
      if (itemModel.children[i].showChildren != undefined && itemModel.children[i].showChildren) {
        return;
      }
    }
    itemModel.children[firstMenu].showChildren = true;
  }

  VisualItemModel {
    id : itemModel

    property int spaceAbove
    property int spaceBelow

    function refresh() {
      var _depth = -myListView.height;

      var _spaceAbove = 0;
      var _spaceBelow = 0;
      var _activeCount = 0;
      var found = false;

      for ( var i = 0; i < children.length; ++i ) {
        if (children[i].visible) {
          children[i].height = actionItemHeight
          _activeCount++;
        }
        if (children[i].columnHeight != undefined)
          children[i].columnHeight = myListView.height
        if (children[i].totalWidth != undefined)
          children[i].totalWidth = _topLevel.width - actionItemWidth
        if (children[i].depth != undefined) {
          children[i].depth = _depth
        }
        _depth += children[i].height + actionItemSpacing;
        if (children[i].actionItemSpacing != undefined)
          children[i].actionItemSpacing = actionItemSpacing
        if (children[i].actionItemHeight != undefined)
          children[i].actionItemHeight = actionItemHeight
        if (children[i].doCollapse != undefined) {
          children[i].doCollapse.disconnect(_topLevel, doCollapse)
          children[i].doCollapse.connect(_topLevel, doCollapse)
        }
        children[i].triggered.disconnect( itemModel, triggered )
        children[i].triggered.connect( itemModel, triggered )
        // children[i].width = parent.actionItemWidth
        if (i == myListView.currentIndex)
        {
          found = true;
          if (children[i].showChildren != undefined)
            children[i].showChildren = true;
        }
        if (found)
          _spaceBelow += children[i].height
        else
          _spaceAbove += children[i].height
      }
      spaceAbove = _spaceAbove;
      if (myListView.currentItem)
        spaceBelow = _spaceBelow - myListView.currentItem.height;
      else
        spaceBelow = _spaceBelow;
      activeCount = _activeCount;
    }
    onChildrenChanged : {
      refresh();
    }
    function triggered(triggeredName) {
      myScriptActions.trigger(triggeredName)
      _topLevel.triggered(triggeredName)

      var _spaceAbove = 0;
      var _spaceBelow = 0;
      var found = false;
      for ( var i = 0; i < children.length; ++i ) {
        if (children[i].name != triggeredName && children[i].showChildren != undefined) {
          children[i].showChildren = false;
        }
        else if (children[i].name == triggeredName)
        {
          found = true
          myListView.currentIndex = i
        }
        if (found)
          _spaceBelow += children[i].height + actionItemSpacing
        else
          _spaceAbove += children[i].height + actionItemSpacing
      }
      spaceAbove = _spaceAbove;
      spaceBelow = _spaceBelow - myListView.currentItem.height;
    }
    function showOnlyCategory(category)
    {
      for ( var i = 0; i < children.length; ++i ) {
        if ( children[i].category != undefined && children[i].category != "standard" )
        {
          children[i].visible = (children[i].category == category)
        }
      }
      refresh();
    }
  }

  KPIM.DecoratedListView {
    height : parent.height
    width : parent.actionItemWidth
    id : myListView
    model : itemModel
    spacing : actionItemSpacing
    interactive: (actionItemHeight + actionItemSpacing) * (activeCount <= 0 ? count : activeCount) > height

    highlight : ActiveActionMenuItemDelegate{
      id : menuHighLight;
      visible : menuStyle;
      spaceAbove : itemModel.spaceAbove;
      spaceBelow : itemModel.spaceBelow;
      actionItemHeight: _topLevel.actionItemHeight 
    }

    onHeightChanged : {
      itemModel.refresh()
    }
  }

  onActionItemSpacingChanged : {
    itemModel.refresh();
  }

  onActionItemHeightChanged : {
    itemModel.refresh();
  }
}
