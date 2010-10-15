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
  property int contentHeight: subject.height + messageContent.height + bottomContainer.height + 20;
  anchors.topMargin: 12
  anchors.leftMargin: 48
  anchors.rightMargin: 2

  Text {
    id: subjectLabel
    text: KDE.i18n( "Subject:" );
    anchors.leftMargin: 48
    anchors.verticalCenter: subject.verticalCenter
    anchors.left: parent.left
  }

  Rectangle {
    id: subject
    anchors {
      left: subjectLabel.right
      top: parent.top
      right: parent.right
    }
    height: subjectInput.height + 8 // padding
    border { color: "grey"; width: 2; }
    radius: 8

    TextInput {
      id: subjectInput
      anchors.fill: parent
      anchors.margins: 4
      text: window.subject
    }
  }

  Binding { target: window; property: "subject"; value: subjectInput.text }

  MessageComposer.Editor {
    id: messageContent
    height: 344;
    anchors {
      top: subject.bottom
      left: parent.left
      right: parent.right
      topMargin: 2
    }
  }

  Item {
    id: bottomContainer;
    height: 80
    anchors {
        left: parent.left
        right: parent.right
        top: messageContent.bottom
    }

    Text {
      id: identityLabel
      anchors {
        left: parent.left
        bottom: parent.bottom
          top: parent.top
      }
      text: KDE.i18n( "Identity:" );
      verticalAlignment: "AlignVCenter"
    }

    KPIMIdentities.IdentityComboBox {
      id: identityCombo
      anchors {
        left: identityLabel.right
        top: parent.top
        bottom: parent.bottom
        right: cancelButton.left
      }
    }

    KPIM.Button2 {
      id: cancelButton
      anchors.bottom: parent.bottom;
      anchors.right: sendButton.left;
      width: height * 1.5;
      height: identityCombo.height
      icon: KDE.iconPath( "dialog-cancel", 64 );
      onClicked: window.close();
    }

    KPIM.Button2 {
      id: sendButton;
      anchors.bottom: parent.bottom;
      anchors.right: parent.right;
      width: height * 1.5;
      height: identityCombo.height
      icon: KDE.iconPath( "mail-folder-outbox", 64 );
      onClicked: window.send();
    }
  }

}
