/*
    Copyright (c) 2013 Michael Bohlender <michael.bohlender@kdemail.net>
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

import QtQuick 1.1
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

KPIM.DecoratedListView {
  id : _top

  property variant checkModel
  property variant navigationModel
  property int currentItemId: -1
  property int currentRow : -1

  focus: true
  clip: true

  onCurrentRowChanged : {
    if (navigationModel != undefined)
      navigationModel.select(currentRow, 3)
  }

  Connections {
    target : navigationModel
    onCurrentRowChanged : {
      currentRow = navigationModel.currentRow
    }
  }

  delegate: PlasmaComponents.ListItem {
    id: _delegate

    property alias color: itemBackground.color

    height: _top.height / 7
    clip: true

    MouseArea {
      anchors.fill: parent
      onClicked: {
        if (navigationModel != undefined) {
          navigationModel.select(model.index, 3)
        } else {
          _delegate.ListView.view.currentIndex = model.index;
          _delegate.ListView.view.parent.currentItemId = model.itemId;
        }
      }
    }

    Rectangle {
      id: itemBackground

      anchors.fill: parent
    }

    PlasmaComponents.Label {
      id: fromLabel

      anchors {
        top : parent.top
        left : parent.left
        right: dateLabel.left
      }

      text : model.from
      elide: "ElideRight"
      font.weight: Font.Light
      color : "#0C55BB"
    }

    PlasmaComponents.Label {
      id: dateLabel

      anchors {
        top: parent.top
        right: parent.right
      }

      text: model.date
      horizontalAlignment: "AlignRight"
      font.weight: Font.Light
      color : "#0C55BB"
    }

    PlasmaExtras.Heading {
      id: subjectLabel

      anchors {
        top: fromLabel.bottom
        left: parent.left
        right: parent.right
      }

      level: 4
      text: model.subject
      elide: "ElideRight"
      color: model.is_unread ? "#E10909" : "#3B3B3B"
    }
  }
}
