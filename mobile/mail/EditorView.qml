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

import QtQuick 1.1
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kpimidentities 4.5 as KPIMIdentities
import org.kde.messagecomposer 4.5 as MessageComposer
import "../mobileui/ScreenFunctions.js" as Screen
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
  id: root
  property int contentHeight: subjectInput.height + messageContent.height + bottomContainer.height + 20;
  property int screenHeight: 480
  anchors.topMargin: 12
  anchors.leftMargin: 48
  anchors.rightMargin: 2

  PlasmaComponents.Label {
    id: subjectLabel

    anchors {
      verticalCenter: subjectInput.verticalCenter
      left: parent.left
    }

    text: KDE.i18n( "Subject:" )
  }

  PlasmaComponents.TextField {
      id: subjectInput

      anchors {
        left: subjectLabel.right
        top: parent.top
        right: parent.right
      }

      text: window.subject
      clearButtonShown: true
    }

  Binding { target: window; property: "subject"; value: subjectInput.text }

  Rectangle {
    id: cryptoIndicator
    visible: window.isSigned || window.isEncrypted
    anchors.top: subjectInput.bottom
    anchors.topMargin: 2
    anchors.left: parent.left
    anchors.right: parent.right
    height: (window.isSigned || window.isEncrypted) ? 20 : 0

    Row {
      anchors.fill: parent
      spacing: 2

      Rectangle {
        id: signedIndicator
        visible: window.isSigned
        color: "#BAF9CE"
        width: (window.isEncrypted ? parent.width / 2 : parent.width)
        height: 20

        Text {
          anchors.fill: parent
          text: KDE.i18n( "Message will be signed" )
          horizontalAlignment: Text.AlignHCenter
        }
      }

      Rectangle {
        id: encryptedIndicator
        visible: window.isEncrypted
        color: "#0080FF"
        width: (window.isSigned ? parent.width / 2 : parent.width)
        height: 20

        Text {
          anchors.fill: parent
          text: KDE.i18n( "Message will be encrypted" )
          horizontalAlignment: Text.AlignHCenter
        }
      }
    }
  }

  MessageComposer.Editor {
    id: messageContent
    availableScreenHeight: root.screenHeight - bottomContainer.height - subjectInput.height - cryptoIndicator.height - cryptoIndicator.anchors.topMargin - root.anchors.topMargin - 2
    anchors {
      top: cryptoIndicator.bottom
      left: parent.left
      right: parent.right
      topMargin: 2
    }
  }

  Item {
    id: bottomContainer;
    height: Screen.fingerSize
    anchors {
        left: parent.left
        right: parent.right
        top: messageContent.bottom
    }

    PlasmaComponents.Label {
      id: identityLabel
      anchors {
        left: parent.left
        bottom: parent.bottom
        top: parent.top
      }
      text: KDE.i18n( "Identity:" )
      verticalAlignment: Text.AlignVCenter
    }

    KPIMIdentities.IdentityComboBox {
      id: identityCombo
      anchors {
        left: identityLabel.right
        top: parent.top
        bottom: parent.bottom
        right: cancelButton.left
      }
      // HACK: the style sheet currently enforces a way to big minimum height on < 200 dpi
      // not trivial to remove there unfortunately, so work around that in places where the layout
      // is already DPI-aware
      styleSheet: styleSheet + " QComboBox { min-height: 0px }"
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
      color: window.tooManyRecipients ? "red" : "#00000000"
      buttonText: window.recipientsCount == 0 ? "" : window.recipientsCount
      onClicked: window.send();
    }
  }

}
