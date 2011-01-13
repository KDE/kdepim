/* -*- mode: javascript; c-basic-offset:2 -*-
    kleopatra-mobile.qml

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.kleopatra 2.1 as Kleo

KPIM.MainView {
  id: kleopatraMobile;

  QML.Rectangle {

    anchors.fill : parent

    Kleo.KeyTreeView {
      id : keyTreeView

      anchors.top    : parent.top
      anchors.bottom : searchBar.top
      anchors.left   : parent.left
      anchors.right  : parent.right
    }

    Kleo.SearchBar {
      id : searchBar

      anchors.bottom : parent.bottom
      anchors.left   : parent.left
      anchors.right  : parent.right

      visible : false
      height  : 0
      y       : height == 0 ? parent.height : parent.height - height
    }

    QML.Rectangle {
      anchors.fill : parent
      visible : !application.certificatesAvailable

      QML.Text {
        text : KDE.i18n( "No certificates loaded yet." );
        anchors.centerIn : parent
      }
    }
  }

  SlideoutPanelContainer {
    anchors.fill: parent

    SlideoutPanel {
      id: actionPanel
      titleText: KDE.i18n( "Actions" )
      handlePosition : 125
      handleHeight: 150
      anchors.fill : parent

      content : [
        KleopatraActions {
          id : kleopatraActions
          anchors.fill : parent

/*
          scriptActions : [
            KPIM.ScriptAction {
              name : "show_about_dialog"
              script : {
                actionPanel.collapse();
                aboutDialog.visible = true;
              }
            },
            KPIM.ScriptAction {
              name : "configure"
              script : {
                actionPanel.collapse();
                configDialog.visible = true;
              }
            },
            KPIM.ScriptAction {
              name : "to_selection_screen"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.MultipleFolderSelectionScreenState );
              }
            },
            KPIM.ScriptAction {
              name : "add_as_favorite"
              script : {
                actionPanel.collapse();
                application.saveFavorite();
              }
            },
            KPIM.ScriptAction {
              name : "start_maintenance"
              script : {
                actionPanel.collapse();
                guiStateManager.pushState( KPIM.GuiStateManager.BulkActionScreenState );
              }
            }
          ]
*/

          onDoCollapse : actionPanel.collapse();
        }
      ]
    }
  }

}
