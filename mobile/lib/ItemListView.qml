/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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
import org.kde 4.5

Item {
  property alias model: itemListView.model
  property alias currentIndex: itemListView.currentIndex
  property int currentItemId: -1
  property alias delegate: itemListView.delegate
  property alias count: itemListView.count

  signal itemSelected

  function nextItem() {
    if ( currentIndex < (model.itemCount - 1) ) {
      currentIndex = currentIndex + 1;
      currentItemId = model.itemId( currentIndex );
      itemSelected();
      application.setListSelectedRow(currentIndex);
    }
  }

  function previousItem() {
    if ( currentIndex > 0  ) {
      currentIndex = currentIndex - 1;
      currentItemId = model.itemId( currentIndex );
      itemSelected();
      application.setListSelectedRow(currentIndex);
    }
  }

  ListView {
    id: itemListView
    anchors.fill: parent
    highlightFollowsCurrentItem: true
    highlightRangeMode: "StrictlyEnforceRange"
    preferredHighlightBegin: height/2 - currentItem.height/2
    preferredHighlightEnd: height/2 + currentItem.height/2
    focus: true
    clip: true
  }
}
