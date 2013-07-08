/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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

import QtQuick 1.1 as QML
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Rectangle {
  color : "#00000000"
  id : _topListView
  property alias model: itemListView.model
  property alias currentIndex: itemListView.currentIndex
  property int currentItemId: -1
  property int currentRow : -1
  property alias delegate: itemListView.delegate
  property alias count: itemListView.count
  property alias section: itemListView.section
  property variant navigationModel
  property int itemHeight: height / 7

  PlasmaExtras.ScrollArea {
    anchors.fill: parent
    flickableItem: QML.ListView {
      id: itemListView

      focus: true
      clip: true
    }
  }
  onCurrentRowChanged : {
    if (navigationModel != undefined)
      navigationModel.select(currentRow, 3)
  }
  QML.Connections {
    target : navigationModel
    onCurrentRowChanged : {
      _topListView.currentRow = navigationModel.currentRow
    }
  }

}
