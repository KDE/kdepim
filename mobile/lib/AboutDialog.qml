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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM

QML.Rectangle {
  id: aboutDialog
  property alias source: backgroundImage.source
  anchors.fill: parent
  color: "white"
  visible: false

  QML.Image {
    id: backgroundImage
    anchors.fill: parent
    anchors.topMargin: 40
    QML.Flickable{
        flickableDirection: QML.Flickable.VerticalFlick
        width: parent.width - closeButton.width
        height: parent.height
        contentHeight: 1200;
        QML.Rectangle{
        id: aboutText
            QML.Column{
                QML.Text{
                    id: caption
                    font.pointSize: 18
                    style: QML.Text.Raised
                    text: "About " + application.name
                }

                QML.Text{
                    id:version
                    horizontalAlignment: QML.Text.AlignHCenter
                    text: "\n" + application.version + "\n"
                }

                QML.Text {
                    text : KDE.i18n("This Free Software product was created as part of a commercial contract.") +
                           "\n" + application.name +
                           KDE.i18n(" is licensed under the GNU GPL version 2 or later." ) + "\n" +
                           KDE.i18n("See") + " licenses.pdf " + KDE.i18n("for details.") + "\n"
                }

                QML.Text{
                    text: KDE.i18n("Credits Project Komo3 (October 2009 - )") + "\n"
                }

                QML.Text{
                    text: KDE.i18n("Scrum Master, Team Senior: Till Adam, KDAB Berlin") + "\n" +
                          KDE.i18n("Product Owner, Team Senior: Bernhard Reiter, Intevation GmbH") + "\n"
                    style: QML.Text.Raised
                }

                QML.Text{
                    id: mainTeam
                    text: KDE.i18n("Main Scrum team in alphabetical order:") + "\n\n" +
                          "\t" + KDE.i18n("Heinecke, Andre (Intevation GmbH)") + "\n" +
                          "\t" + KDE.i18n("Ricks, Björn (Intevation GmbH)") + "\n" +
                          "\t" + KDE.i18n("Teichmann, Sascha L. (Intevation GmbH)") + "\n"
                }

                QML.Text{
                    id: satellites
                    text: KDE.i18n("\"Satellites\" and additional support by:") + "\n\n" +
                          "\t" + KDE.i18n("Reiter, Ludwig (Intevation GmbH)") + "\n" +
                          "\t" + KDE.i18n("Wolfsteller, Felix (Intevation GmbH)") + "\n"
                }

                QML.Text{
                    style: QML.Text.Raised
                    text: KDE.i18n("Special thanks to the two project persons from our principal.") + "\n"
                }

                QML.Text{
                    text: KDE.i18n("This project is built upon KDE SC Kontact Desktop\n" +
                                   "client. For the outstanding work done there we\n" +
                                   "would like to thank the original authors.\n")
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
            aboutDialog.visible = false
        }
    }
}
