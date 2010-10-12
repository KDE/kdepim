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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.akonadi.mail 4.5 as Mail

Rectangle {
  id: configDialog
  anchors.fill: parent
  z: 10
  color: "white"

  Flickable {
    id: configWidgetBox
    anchors.fill: parent
    anchors.topMargin: 25
    flickableDirection: Flickable.VerticalFlick
    contentHeight: configWidget.height;

    Mail.ConfigWidget {
      id: configWidget

      onVisibleChanged: {
        if ( visible ) {
          configWidget.load()
        }
      }
    }
  }

  KPIM.Button2 {
    id: okButton
    anchors.top: parent.top
    anchors.topMargin: 20
    anchors.right: parent.right
    width: 150
    buttonText: KDE.i18n( "Ok" )
    onClicked: {
      configWidget.save();
      configDialog.visible = false
    }
  }

  KPIM.Button2 {
    id: cancelButton
    anchors.top: okButton.bottom
    anchors.right: parent.right
    width: 150
    buttonText: KDE.i18n( "Cancel" )
    onClicked: {
      configDialog.visible = false
    }
  }
}
