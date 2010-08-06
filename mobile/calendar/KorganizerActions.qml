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
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  actionItemHeight : 70
  actionItemWidth : 200
  actionItemSpacing : 2
  ReorderList {
    category : "home"
    name : "Favorites"

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "fav_up"
    downAction : "fav_down"
    deleteAction : "fav_delete"
    model : favoritesList
  }
  ReorderList {
    category : "home"
    name : "Accounts"

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "acc_up"
    downAction : "acc_down"
    deleteAction : "acc_delete"
    model : allFoldersModel
  }
  ActionList {
    category : "home"
    name : "Sources"
    ActionListItem { name : "to_selection_screen" }
  }

  ActionList {
    category : "account"
    name : "Account"
    ActionListItem { name : "edit_account" }
    ActionListItem { name : "set_as_default" }
    ActionListItem { name : "add_subfolder" }
  }
  /*
  ActionList {
    category : "account"
    name : "Export"
    ActionListItem { name : "export_to_ical" }
  } */

  ActionList {
    category : "single_folder"
    name : "Folder"
    ActionListItem { name : "edit_folder" }
    ActionListItem { name : "delete_folder" }
    ActionListItem { name : "add_subfolder" }
  }
  ActionList {
    category : "single_folder"
    name : "Events"
    ActionListItem { name : "start_maintenance" }
  }

  ActionList {
    category : "multiple_folder"
    name : "Sources"

    //
    ActionListItem { name : "change_folder_selection" }
  }
  ActionList {
    category : "multiple_folder"
    name : "Events"
    ActionListItem { name : "start_maintenance" }
  }

  ActionList {
    category : "single_calendar"
    name : "Favorite"
    ActionListItem { name : "single_add_as_favorite" }
    ActionListItem { name : "single_remove_from_favorites" }
  }
  ActionList {
    category : "single_calendar"
    name : "View"
    ActionListItem { name : "view_day" }
    ActionListItem { name : "view_week" }
    ActionListItem { name : "view_month" }
  }
  ActionList {
    category : "single_calendar"
    name : "Date"
    ActionListItem { name : "goto_today" }
    ActionListItem { name : "select_date" }
  }
  ActionList {
    category : "single_calendar"
    name : "Publish"
    ActionListItem { name : "publish_as_webpage" }
    ActionListItem { name : "publish_as_ical" }
    ActionListItem { name : "publish_as_vcal" }
  }

  ActionList {
    category : "multiple_calendar"
    name : "Favorite"
    ActionListItem { name : "mult_add_as_favorite" }
    ActionListItem { name : "mult_remove_from_favorites" }
  }
  ActionList {
    category : "multiple_calendar"
    name : "View"
    ActionListItem { name : "view_day" }
    ActionListItem { name : "view_week" }
    ActionListItem { name : "view_month" }
  }
  ActionList {
    category : "multiple_calendar"
    name : "Date"
    ActionListItem { name : "goto_today" }
    ActionListItem { name : "select_date" }
  }
  ActionList {
    category : "multiple_calendar"
    name : "Publish"
    ActionListItem { name : "publish_as_webpage" }
    ActionListItem { name : "publish_as_ical" }
    ActionListItem { name : "publish_as_vcal" }
  }

  ActionList {
    category : "event_viewer"
    name : "Event"
    ActionListItem { name : "copy_to_calendar" }
    ActionListItem { name : "move_to_calendar" }
  }
  ActionList {
    category : "event_viewer"
    name : "Schedule"
    ActionListItem { name : "publish_item_information" }
    ActionListItem { name : "send_information_to_attendees" }
    ActionListItem { name : "send_cancellation_requests" }
    ActionListItem { name : "request_update" }
    ActionListItem { name : "request_change" }
    ActionListItem { name : "send_as_ical" }
    ActionListItem { name : "mail_free_busy" }
    ActionListItem { name : "upload_free_busy" }
  }
  ActionList {
    category : "event_viewer"
    name : "Flag"
    ActionListItem { name : "toggle_read_unread" }
  }
  ActionList {
    category : "event_viewer"
    name : "Publish"
    ActionListItem { name : "publish_as_webpage" }
    ActionListItem { name : "publish_as_ical" }
    ActionListItem { name : "publish_as_vcal" }
  }
  ApplicationGeneralActions {
    category : "standard"
    name : "Korganizer"
    type : "events"
  }
}
