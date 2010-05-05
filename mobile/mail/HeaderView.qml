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

import Qt 4.7 as QML
import org.kde.pim.mobileui 4.5 as KPIM

/** Akonadi Message Header List View
 */
KPIM.ItemListView {
  delegate: [
    KPIM.ItemListViewDelegate {
      summaryContent: [
        QML.Text {
          anchors.fill: parent
          text: model.subject
        }
      ]
      detailsContent: [
        QML.Column {
          anchors.fill: parent
          QML.Text {
            text: model.subject
            font.bold: true
            color: palette.highlightedText
          }
          QML.Text {
            text: "From: " + model.from
            color: palette.highlightedText
          }
          QML.Text {
            text: "Date: " + model.date
            color: palette.highlightedText
          }
        }
      ]
    }
  ]
}
