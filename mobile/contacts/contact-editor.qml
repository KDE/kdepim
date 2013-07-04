/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

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
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.contacteditors 4.5 as ContactEditors

KPIM.MainView {
  KPIM.DecoratedFlickable {
    anchors.fill: parent

    anchors.topMargin: 40
    anchors.leftMargin: 40;
    anchors.rightMargin: 4;

    contentHeight: editorGeneral.height;

    content.children: [
      Item {
        anchors.fill: parent
        ContactEditors.ContactEditorGeneral {
          id: editorGeneral;
          width: parent.width;
        }
      }
    ]
 }

  SlideoutPanelContainer {
    anchors.fill: parent
    z: 50

    SlideoutPanel {
      anchors.fill: parent
      id: businessPanel
      titleText: KDE.i18n( "Business" )
      handlePosition: 30
      handleHeight: 120

      content: [
        KPIM.DecoratedFlickable {
          anchors.fill: parent;
          contentHeight: editorBusiness.height;

          content.children: [
            Column {
              anchors.fill: parent
              ContactEditors.ContactEditorBusiness {
                id: editorBusiness
                width: parent.width;
              }
            }
          ]
        }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      id: locationPanel
      titleText: KDE.i18n( "Location" )
      handlePosition: 30 + 120
      handleHeight: 120

      content: [
        KPIM.DecoratedFlickable {
          anchors.fill: parent;
          contentHeight: editorLocation.height;

          content.children: [
            Column {
              anchors.fill: parent
              ContactEditors.ContactEditorLocation {
                id: editorLocation
                width: parent.width;
              }
            }
          ]
        }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      id: cryptoPanel
      titleText: KDE.i18n( "Crypto" )
      handlePosition: 30 + 120 + 120
      handleHeight: 100

      content: [
        KPIM.DecoratedFlickable {
          anchors.fill: parent;
          contentHeight: editorCrypto.height;

          content.children: [
            Column {
              anchors.fill: parent
              ContactEditors.ContactEditorCrypto {
                id: editorCrypto
                width: parent.width;
              }
            }
          ]
        }
      ]
    }
    SlideoutPanel {
      anchors.fill: parent
      id: morePanel
      titleText: KDE.i18n( "More" )
      handlePosition: 30 + 120 + 120 + 100
      handleHeight: 100

      content: [
        KPIM.DecoratedFlickable {
          anchors.fill: parent;
          contentHeight: editorMore.height;

          content.children: [
            Column {
              anchors.fill: parent
              ContactEditors.ContactEditorMore {
                id: editorMore
                width: parent.width;
              }
            }
          ]
        }
      ]
    }
  }
}
