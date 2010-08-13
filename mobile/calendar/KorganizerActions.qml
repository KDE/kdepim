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
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  actionItemHeight : 70
  actionItemWidth : 200
  actionItemSpacing : 2
  ReorderList {
    category : "home"
    name : KDE.i18n( "Favorites" )

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "fav_up"
    downAction : "fav_down"
    deleteAction : "fav_delete"
    model : favoritesList
  }
  AgentInstanceList {
    category : "home"
    name : KDE.i18n( "Calendars" )

    model : agentInstanceList

    customActions : [
      ActionListItem { name : "akonadi_agentinstance_delete" },
      ActionListItem { name : "akonadi_agentinstance_configure" },
      ActionListItem { name : "akonadi_agentinstance_create" }
    ]
  }
  ActionList {
    category : "home"
    name : KDE.i18n( "Sources" )
    FakeAction { name : "to_selection_screen" }
  }

  ActionList {
    category : "account"
    name : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_collection_move_to_menu" }
    ActionListItem { name : "akonadi_collection_copy_to_menu" }
    ActionListItem { name : "akonadi_collection_delete" }
  }
  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Events" )
    ActionListItem { name : "start_maintenance" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "Sources" )

    //
    ActionListItem { name : "change_folder_selection" }
  }
  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "Events" )
    ActionListItem { name : "start_maintenance" }
  }

  ActionList {
    category : "single_calendar"
    name : KDE.i18n( "Favorite" )
    ActionListItem { name : "single_add_as_favorite" }
    ActionListItem { name : "single_remove_from_favorites" }
  }
  ActionList {
    category : "single_calendar"
    name : KDE.i18n( "View" )
    FakeAction { name : "view_day" }
    FakeAction { name : "view_week" }
    FakeAction { name : "view_month" }
    FakeAction { name : "back_to_folder_selection" }
  }
  ActionList {
    category : "single_calendar"
    name : KDE.i18n( "Date" )
    FakeAction { name : "goto_today" }
    FakeAction { name : "select_date" }
  }

  ActionList {
    category : "multiple_calendar"
    name : KDE.i18n( "Favorite" )
    FakeAction { name : "mult_add_as_favorite" }
    FakeAction { name : "mult_remove_from_favorites" }
  }
  ActionList {
    category : "multiple_calendar"
    name : KDE.i18n( "View" )
    FakeAction { name : "view_day" }
    FakeAction { name : "view_week" }
    FakeAction { name : "view_month" }
    FakeAction { name : "back_to_folder_selection" }
  }
  ActionList {
    category : "multiple_calendar"
    name : KDE.i18n( "Date" )
    FakeAction { name : "goto_today" }
    FakeAction { name : "select_date" }
  }

  ActionList {
    category : "event_viewer"
    name : KDE.i18n( "Event" )
    ActionListItem { name : "akonadi_item_copy_to_menu" }
    ActionListItem { name : "akonadi_item_move_to_menu" }
    ActionListItem { name : "akonadi_item_delete" }
    FakeAction { name : "akonadi_edit_event" }
    FakeAction { name : "detach_recurring_event" } // is this even available on the desktop?
  }
  ActionList {
    category : "event_viewer"
    name : KDE.i18n( "Schedule" )
    FakeAction { name : "publish_item_information" }
    FakeAction { name : "send_information_to_attendees" }
    FakeAction { name : "send_cancellation_requests" }
    FakeAction { name : "request_update" }
    FakeAction { name : "request_change" }
    FakeAction { name : "send_as_ical" }
    FakeAction { name : "mail_free_busy" }
    FakeAction { name : "upload_free_busy" }
  }
  ActionList {
    category : "event_viewer"
    name : KDE.i18n( "Attachments" )
    FakeAction { name : "save_all" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : KDE.i18n( "KOrganizer" )
    type : "event"
  }
}
