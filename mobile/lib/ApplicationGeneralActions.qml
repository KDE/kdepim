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


KPIM.ActionList {
  id : actions
  property alias configureActionVisible: configureAction.visible
  property string type
  property alias addNewActionName: addNewAction.name
  property alias addNewActionReactsOnLongPressed: addNewAction.reactsOnLongPressed
  property alias searchActionTitle: searchAction.title
  property alias configureActionTitle: configureAction.title
  signal longPressed(string actionName)

  KPIM.ActionListItem {
    name : "quit"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ActionListItem {
    name : "quit_akonadi"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ActionListItem {
    id: addNewAction
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ScriptActionItem {
    id : searchAction
    name : "search_" + type
  }

  KPIM.ActionListItem {
    name : "akonadi_work_offline"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ScriptActionItem {
    id : configureAction
    name : "configure"
  }

  KPIM.ActionListItem {
    name : "open_manual"
    title: KDE.i18n( "First Steps" )
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ScriptActionItem  {
    name : "show_about_dialog"
    title : KDE.i18n( "About Kontact Touch" )
  }
}

