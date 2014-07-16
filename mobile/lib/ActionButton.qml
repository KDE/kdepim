/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

import QtQuick 1.1
import org.kde.pim.mobileui 4.5 as KPIM

/**
 * @short A button that is connected with a QAction
 *
 * The button will update its enabled state according to the
 * associated action.
 */
KPIM.Button {
  property variant action
  property string actionName

  width: 70
  height: 70
  action: application.getAction( actionName, "" )
  enabled: action.enabled
  opacity: enabled ? 1 : 0.65

  onClicked : { action.trigger() }

  Connections {
    target : action
    onChanged : {
      border.width = action.checked ? 2 : 0
    }
  }

  border.width : action.checked ? 2 : 0
}
