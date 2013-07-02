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

import Qt 4.7
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.contacteditors 4.5 as ContactEditors
import org.kde.plasma.extras 0.1 as PlasmaExtras

KPIM.MainView {
  PlasmaExtras.ScrollArea {
    anchors {
      fill: parent
      topMargin: 40
      leftMargin: 40;
      rightMargin: 4;
    }

    flickableItem: Flickable {
      contentHeight: editorGeneral.height;

      ContactEditors.ContactEditorGeneral {
        id: editorGeneral;
        width: parent.width;
      }
    }
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
      PlasmaExtras.ScrollArea {
        anchors.fill: parent;

        flickableItem: Flickable {
          contentHeight: editorBusiness.height;

          ContactEditors.ContactEditorBusiness {
            id: editorBusiness
            width: parent.width;
          }
        }
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
      PlasmaExtras.ScrollArea {
        anchors.fill: parent;

        flickableItem: Flickable {
          contentHeight: editorLocation.height;

          ContactEditors.ContactEditorLocation {
            id: editorLocation
            width: parent.width;
          }
        }
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
      PlasmaExtras.ScrollArea {
        anchors.fill: parent;

        flickableItem: Flickable {
          contentHeight: editorCrypto.height;

          ContactEditors.ContactEditorCrypto {
            id: editorCrypto
            width: parent.width;
          }
        }
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
      PlasmaExtras.ScrollArea {
        anchors.fill: parent;

        flickableItem: Flickable {
          contentHeight: editorMore.height;

          ContactEditors.ContactEditorMore {
            id: editorMore
            width: parent.width;
          }
        }
      }
      ]
    }
  }
}
