/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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
import org.kde.akonadi.mail 4.5 as Mail

QML.Rectangle {
  id: filterConfigDialog
  anchors.fill: parent
  z: 10
  color: "white"

  property alias filterModel: filterList.model

  QML.Rectangle {
    id: list
//     color: "red"
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: 250

    QML.Text {
      id: topText
      color: black
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.topMargin: 35
      anchors.rightMargin: 25
      verticalAlignment: QML.Text.AlignVCenter
      text : KDE.i18n( "Filters" )
    }
    QML.Image {
      id : bottomLine
      source : "dividing-line-horizontal.png"
      anchors.right : parent.right
      anchors.top: topText.bottom
      anchors.topMargin: 15
      fillMode : QML.Image.TileHorizontally
      width : parent.width - 15
    }
    QML.Image {
      source : "dividing-line.png"
      anchors.top : bottomLine.bottom
      anchors.right : parent.right
      height : filterList.height - 15
      fillMode : QML.Image.TileVertically
    }

    QML.ListView {
      id: filterList
      anchors.topMargin: 25
      anchors.top: topText.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.leftMargin: 5
      anchors.rightMargin: 5
      clip:true
      focus: true
      interactive: count* 45 > height

      highlight: QML.Rectangle { border.color: "blue"; radius: 5 }

      delegate: QML.Text {
        height: 45
        verticalAlignment: QML.Text.AlignVCenter
        horizontalAlignment: QML.Text.AlignLeft
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 5
        text: display
        QML.MouseArea {
          anchors.fill: parent
          onClicked: { filterList.currentIndex = model.index; }
        }
     }
    onCurrentIndexChanged: {
        console.log( "Filter selected", currentIndex );
        configWidget.loadFilter( currentIndex )
      }
    }
  }

  QML.Flickable {
    id: configWidgetBox
    anchors.left: list.right
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    //anchors.topMargin: 25
    flickableDirection: QML.Flickable.VerticalFlick
    contentHeight: configWidget.height;

    QML.Item { // dummy item to make the widget visible with the broken QML version on the N900
      anchors.fill: parent 
      Mail.FilterConfigWidget {
        id: configWidget
        width: parent.width - okButton.width

        onVisibleChanged: {
          if ( visible ) {
            //configWidget.load()
          }
        }
      }
    }
  }

  QML.Rectangle {
    id : backToMessageListButton
    visible : true
    anchors.right : filterConfigDialog.right
    anchors.rightMargin : 70
    anchors.bottom : filterConfigDialog.bottom
    anchors.bottomMargin : 100
    QML.Image {
      source : KDE.locate( "data", "mobileui/back-to-list-button.png" );
      QML.MouseArea {
        anchors.fill : parent;
        onClicked : {
          configWidget.save();
          filterConfigDialog.visible = false
       }
      }
    }
  }
}
