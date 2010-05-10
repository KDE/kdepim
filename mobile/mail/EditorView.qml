/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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
import org.kde.kpimidentities 4.5 as KPIMIdentities
import org.kde.messagecomposer 4.5 as MessageComposer

Item {
  anchors.topMargin: 12
  anchors.leftMargin: 48
  anchors.rightMargin: 2

  Text {
    id: subjectLabel
    text: KDE.i18n( "Subject:" );
    anchors.leftMargin: 48
    anchors.top: parent.top
    anchors.left: parent.left
  }

  Rectangle {
    id: subject
    anchors {
      left: subjectLabel.right
      top: parent.top
      right: parent.right
    }
    height: subjectInput.height
    border { color: "black"; width: 1 }

    TextInput {
      id: subjectInput
      anchors.fill: parent
    }
  }

  MessageComposer.Editor {
    id: messageContent
    anchors {
      top: subject.bottom
      left: parent.left
      right: parent.right
      bottom: sendButton.top
      topMargin: 2
    }
  }

  Text {
    id: identityLabel
    anchors {
      left: parent.left
      bottom: parent.bottom
      top: messageContent.bottom
    }
    text: KDE.i18n( "Identity:" );
    verticalAlignment: "AlignVCenter"
  }

  KPIMIdentities.IdentityComboBox {
    id: identityCombo
    anchors {
      left: identityLabel.right
      bottom: parent.bottom
      top: messageContent.bottom
      right: sendButton.left
    }
    height: parent.height / 6
  }

  KPIM.Button {
    id: sendButton
    anchors {
      bottom: parent.bottom
      right: parent.right
    }
    buttonText: KDE.i18n( "Send" )
    height: parent.height / 6
    width: parent.width / 4
    onClicked: window.send()
  }
}
