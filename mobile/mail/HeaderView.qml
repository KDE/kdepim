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
          id : summaryText
          anchors { left : parent.left; bottom : parent.bottom; top : parent.top; }
          text: model.subject
        },
        QML.Image {
          id : newFlagImage
          anchors { top : parent.top; bottom : parent.bottom; right : importantFlagImage.left }
          width : { (model.is_new || model.isUnread) ? parent.height - 2 : 0 }
          source : KDE.iconPath(model.is_new ? "mail-unread-new.png" : "mail-unread.png", parent.height - 2)
        },
        QML.Image {
          id : importantFlagImage
          anchors { top : parent.top; bottom : parent.bottom; right : actionFlagImage.left }
          width : { model.is_important ? parent.height - 2 : 0 }
          source : KDE.iconPath("emblem-important.png", parent.height - 2)
        },
        QML.Image {
          id : actionFlagImage
          anchors { top : parent.top; bottom : parent.bottom; right : parent.right }
          width : { model.is_action_item ? parent.height - 2 : 0 }
          source : KDE.iconPath("mail-mark-task.png", parent.height - 2)
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
            text: KDE.i18na( "From: %1",  [model.from] )
            color: palette.highlightedText
          }
          QML.Text {
            text: KDE.i18na( "Date: %1",  [model.date] )
            color: palette.highlightedText
          }
        }
      ]
    }
  ]
}
