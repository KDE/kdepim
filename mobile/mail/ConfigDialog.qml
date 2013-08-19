/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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
import org.kde.akonadi.mail 4.5 as Mail
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Rectangle {
  id: configDialog
  anchors.fill: parent
  z: 10
  color: "white"
  visible: guiStateManager.inConfigScreenState

  function load()
  {
    configWidget.load();
  }

  PlasmaExtras.ScrollArea {
    id: configWidgetBox
    anchors {
      top: parent.top
      topMargin: 25
      bottom: parent.bottom
      left: parent.left
      right: okButton.left
    }

    flickableItem: QML.Flickable {
      contentHeight: configWidget.height;

      contentItem.children: [
      QML.Item { // dummy item to make the widget visible with the broken QML version on the N900
        anchors.fill: parent 
        Mail.ConfigWidget {
          id: configWidget
          width: parent.width
        }
      }
      ]
    }
  }

  KPIM.Button2 {
    id: okButton
    anchors.top: parent.top
    anchors.topMargin: 20
    anchors.right: parent.right
    width: 150
    buttonText: KDE.i18n( "OK" )
    onClicked: {
      configWidget.save();
      guiStateManager.popState()
    }
  }

  KPIM.Button2 {
    id: cancelButton
    anchors.top: okButton.bottom
    anchors.right: parent.right
    width: 150
    buttonText: KDE.i18n( "Cancel" )
    onClicked: {
      guiStateManager.popState()
    }
  }
}
