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


KPIM.ActionList {
  id : actions

  property string type
  signal longPressed(string actionName)

  KPIM.ActionListItem {
    name : "quit"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ActionListItem {
    name : "wm_task_switch"
    onPressAndHold: {
      longPressed(name);
    }
  }
  KPIM.ActionListItem {
    name : "add_new_" + type
    onPressAndHold: {
      longPressed(name);
    }
  }
  // TODO: implement us!
/*  KPIM.FakeAction {
    name : "search_" + type
  }
*/
  KPIM.ActionListItem {
    name : "akonadi_work_offline"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ScriptActionItem {
    name : "configure"
    title : KDE.i18n( "Configure" )
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ActionListItem {
    name : "report_bug"
    onPressAndHold: {
      longPressed(name);
    }
  }

  KPIM.ScriptActionItem  {
    name : "show_about_dialog"
    title : KDE.i18n( "About" )
    onPressAndHold: {
      longPressed(name);
    }
  }

  QML.Component.onCompleted :
  {
    for (var i = 0; i < children.length; ++i )
    {
      children[i].triggered.connect(actions, triggered);
    }
  }
}

