/*
 *    Copyright (C) 2013 Michael Bohlender <michael.bohlender@kdemail.net>
 *    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
 *        a KDAB Group company, info@kdab.net,
 *        author Stephen Kelly <stephen@kdab.com>
 *
 *    This library is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU Library General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or (at your
 *    option) any later version.
 *
 *    This library is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *    License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to the
 *    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *    02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponets
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaComponets.Page {

  implicitWidth: pageRow.width * 2 /3

  tools: PlasmaComponets.ToolBarLayout {
    PlasmaComponets.ToolButton {
      anchors.left: parent.left
      iconSource: "go-previous"
      onClicked: pageRow.pop()
    }
  }

  PlasmaExtras.ScrollArea {

    anchors.fill: parent

    flickableItem: Flickable {
          anchors.fill: parent

      contentHeight: 1000;
      clip: true

      Column{
        id: _content
        anchors.fill: parent
        anchors.margins: 30
        PlasmaExtras.Title{
          id: caption
          text: KDE.i18n( "About %1", application.name )
          wrapMode: Text.WordWrap
          width: parent.width
        }
        PlasmaComponets.Label{
          id:version
          text: "\n" + application.version + "\n"
          wrapMode: Text.WordWrap
          width: parent.width
        }

        PlasmaExtras.Heading  {
          text: KDE.i18n("Contributors")
        }

        PlasmaExtras.Paragraph {
          text: KDE.i18n("Development team in alphabetical order:") + "\n\n" +
          "\t" + KDE.i18n("Andras Mantia (KDAB)") + "\n" +
          "\t" + KDE.i18n("Andre Heinecke (Intevation)") + "\n" +
          "\t" + KDE.i18n("Andreas Holzammer (KDAB)") + "\n" +
          "\t" + KDE.i18n("Bernhard Reiter, Intevation") + "\n" +
          "\t" + KDE.i18n("Bertjan Broeksema (KDAB)") + "\n" +
          "\t" + KDE.i18n("Björn Balazs (Apliki)") + "\n" +
          "\t" + KDE.i18n("Björn Ricks (Intevation)") + "\n" +
          "\t" + KDE.i18n("Casey Link (KDAB)") + "\n" +
          "\t" + KDE.i18n("David Faure (KDAB)") + "\n" +
          "\t" + KDE.i18n("Felix Wolfsteller (Intevation)") + "\n" +
          "\t" + KDE.i18n("Kevin Krammer (KDAB)") + "\n" +
          "\t" + KDE.i18n("Kevin Ottens (KDAB)") + "\n" +
          "\t" + KDE.i18n("Laurent Montel (KDAB)") + "\n" +
          "\t" + KDE.i18n("Leo Franchi (KDAB)") + "\n" +
          "\t" + KDE.i18n("Ludwig Reiter (Intevation)") + "\n" +
          "\t" + KDE.i18n("Marc Mutz (KDAB)") + "\n" +
          "\t" + KDE.i18n("Marcus Brinkmann (g10 Code)") + "\n" +
          "\t" + KDE.i18n("Michael Bohlender (KDE)") + "\n" +
          "\t" + KDE.i18n("Nuno Pinheiro (KDAB)") + "\n" +
          "\t" + KDE.i18n("Patrick Spendrin (KDAB)") + "\n" +
          "\t" + KDE.i18n("Romain Pokrzywka (KDAB)") + "\n" +
          "\t" + KDE.i18n("Sabine Faure (KDAB)") + "\n" +
          "\t" + KDE.i18n("Sascha L. Teichmann (Intevation)") + "\n" +
          "\t" + KDE.i18n("Sergio Martins (KDAB)") + "\n" +
          "\t" + KDE.i18n("Stephen Kelly (KDAB)") + "\n" +
          "\t" + KDE.i18n("Till Adam (KDAB)") + "\n" +
          "\t" + KDE.i18n("Tobias Koenig (KDAB)") + "\n" +
          "\t" + KDE.i18n("Volker Krause (KDAB)") + "\n" +
          "\t" + KDE.i18n("Werner Koch (g10 Code)") + "\n"
          wrapMode: Text.WordWrap
          width: parent.width
        }

        PlasmaExtras.Paragraph {
          text : KDE.i18n("Kontact Touch is licensed under the GNU GPL version 2 or later and other Free Software licenses. " +
          "See %1 for details.", "licenses.pdf") + "\n"
          wrapMode: Text.WordWrap
          width: parent.width
        }

        PlasmaComponets.Button {
          id: licenseButton

          text: i18n( "Show Licenses" )
          onClicked: application.openLicenses()
        }
      }
    }
  }
}
