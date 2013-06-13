/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <amantia@kdab.com>

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
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Page {

  anchors.fill: parent

  tools: PlasmaComponents.ToolBarLayout {
    PlasmaComponents.ToolButton {

      anchors.left: parent.left

      iconSource: "go-previous"

      onClicked: pageStack.pop()
    }
  }

  QML.Rectangle {
    anchors.right : parent.right
    anchors.rightMargin : 70
    anchors.left : parent. left
    anchors.leftMargin : 70
    anchors.top : parent.top
    anchors.topMargin : 70
    color: "lightgray"

    QML.Column {
      anchors.fill: parent
      KPIM.DecoratedListView {
        anchors.fill: parent
        model: _emailTemplateModel
        focus: true
        delegate: [
          KPIM.Button2 {
            width: parent.width
            buttonText : display
            onClicked : {
              application.newMessageFromTemplate( index );
              pageStack.pop()
            }
          }
        ]
      }
    }

    QML.Component.onCompleted: {
      height = Math.min( application.emailTemplateCount(), 5 ) * 52; //ask on the fly, can change
    }

  }
}
