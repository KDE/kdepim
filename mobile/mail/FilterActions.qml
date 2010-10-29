/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (C) 2010 Andras Mantia <andras@kdab.com>

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
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

//   menuStyle : true

  actionItemHeight : height / 6 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ScriptActionItem { name : "new_filter"; title: KDE.i18n( "New Filter" ) }
  ScriptActionItem { name : "delete_filter"; title: KDE.i18n( "Remove Filter" ) }
  ScriptActionItem { name : "rename_filter"; title: KDE.i18n( "Rename Filter" ) }
  ScriptActionItem { name : "move_up_filter"; title: KDE.i18n( "Move Up" ) }
  ScriptActionItem { name : "move_down_filter"; title: KDE.i18n( "Move Down" ) }
}
