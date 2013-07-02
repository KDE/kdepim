/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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
import org.kde.plasma.extras 0.1 as PlasmaExtras

QML.Rectangle {
  id: aboutDialog
  property alias backgroundSource: backgroundImage.source
  anchors.fill: parent
  color: "white"

  QML.Image {
    id: backgroundImage
    anchors.fill: parent
  }

  QML.Item {
    anchors.fill: parent
    anchors.topMargin: 40
    anchors.leftMargin: 40
    PlasmaExtras.ScrollArea {
      width: parent.width - closeButton.width
      height: parent.height

      flickableItem: QML.Flickable {

        contentWidth: width - 10
        contentHeight: 1900;


        QML.Rectangle{
          id: aboutText
          anchors.fill: parent
          QML.Column{
            anchors.fill: parent
            QML.Text{
              id: caption
              font.pointSize: 18
              style: QML.Text.Raised
              text: KDE.i18n( "About %1", application.name )
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Image {
              id: platformLogo
              source: KDE.locate( "data", "mobileui/built-on-the-kde-platform.png" )
            }

            QML.Text{
              id:version
              text: "\n" + application.version + "\n"
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text {
              text : KDE.i18n("This Free Software product was created as part of a commercial contract. " +
              "Kontact Touch is licensed under the GNU GPL version 2 or later and other Free Software licenses. " +
              "See %1 for details.", "licenses.pdf") + "\n"
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              text: KDE.i18n("Credits Project Komo3 (October 2009 - January 2011)") + "\n"
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              text: KDE.i18n("Scrum Master, Team Senior: Till Adam, KDAB") + "\n" +
              KDE.i18n("Product Owner, Team Senior: Bernhard Reiter, Intevation") + "\n"
              style: QML.Text.Raised
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              text: KDE.i18n("Development team in alphabetical order:") + "\n\n" +
              "\t" + KDE.i18n("Andras Mantia (KDAB)") + "\n" +
              "\t" + KDE.i18n("Andre Heinecke (Intevation)") + "\n" +
              "\t" + KDE.i18n("Andreas Holzammer (KDAB)") + "\n" +
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
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              style: QML.Text.Raised
              text: KDE.i18n("Special thanks to the two project persons from our principal.") + "\n"
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              text: KDE.i18n("This product is built upon the KDE Platform 4 and the " +
              "Kontact Desktop Client. " +
              "Thanks to all contributors!\n")
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }

            QML.Text{
              textFormat: QML.Text.RichText
              text: KDE.i18n("For more information and to give feedback<br/>\n" +
              "please take a look at " +
              "<a href=\"http://userbase.kde.org/Kontact_Touch\">Kontact Touch at KDE Userbase</a>")
              wrapMode: QML.Text.WordWrap
              width: parent.width
            }
          }
        }

      }
    }
  }

  KPIM.Button2 {
    id: licenseButton
    anchors.right: parent.right
    anchors.bottom: closeButton.top
    anchors.bottomMargin: 10
    width: 150
    buttonText: KDE.i18n( "Licenses" )
    onClicked: {
      application.openLicenses()
    }
  }

  KPIM.Button2 {
    id: closeButton
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    width: 150
    buttonText: KDE.i18n( "Close" )
    onClicked: {
      aboutDialog.parent.visible = false
    }
  }
}
