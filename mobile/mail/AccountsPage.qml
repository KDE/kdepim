/*
 *  Copyright 2013  Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras
import org.kde.pim.mobileui 4.5 as KPIM
import org.kde.qtextracomponents 0.1 as QtExtra


PlasmaComponents.Page {
  id: root

  implicitWidth: pageRow.width * 2 /3

  //BEGIN: Tools
  tools: PlasmaComponents.ToolBarLayout {
    PlasmaComponents.ToolButton{
      iconSource: "preferences-system"

      onClicked: pageRow.push(Qt.createComponent("SettingsPage.qml") )
    }
  }
  //END: Tools

  ListView {
    id: listView

    anchors.fill: parent

    clip: true

    model: agentInstanceList

    //BEGIN: Delegate
    delegate : PlasmaComponents.ListItem {
      id: listItem

      height: root.height * 0.12

      enabled: true
      checked: listView.currentIndex == index

      //TODO create FoldersListPage.qml
      onClicked: {
        pageRow.pop(root)
        pageRow.push(Qt.createComponent("FolderPage.qml"))
      }

      QtExtra.QIconItem {
        id: iconItem

        anchors {
          verticalCenter: parent.verticalCenter
        }

        height: parent.height
        width: height

       icon: QIcon("application-x-smb-server")

      }

      PlasmaExtras.Heading {
        id: textItem

        anchors {
          left: iconItem.right
          verticalCenter: parent.verticalCenter
        }

        level: 4
        text: model.display
      }

    }
    //END: Delegate

    //BEGIN: Footer
    footer: PlasmaComponents.ListItem {

      height: root.height * 0.12

      enabled: true

      onClicked: application.launchAccountWizard()

      //FIXME show a black plus button instead of the green one?
      QtExtra.QIconItem {
        id: iconItem

        anchors {
          verticalCenter: parent.verticalCenter
        }

        height: parent.height
        width: height

       icon: QIcon("list-add")

      }

      PlasmaExtras.Heading {
        id: textItem

        anchors {
          left: iconItem.right
          verticalCenter: parent.verticalCenter
        }

        text: i18n("Add Account")
        level: 4
      }
    }
   //END: Footer
  }
}