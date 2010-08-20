/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

  actionItemHeight : height / 6 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : KDE.i18n( "Home" )
    ActionListItem { name : "synchronize_all_items" }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select multiple folders" ) }
    FakeAction { name : "purge_completed_todos" }
  }

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
    name : KDE.i18n( "Accounts" )

    model : agentInstanceList

    customActions : [
      ActionListItem { name : "akonadi_agentinstance_configure" },
      ActionListItem { name : "akonadi_agentinstance_delete" },
      ActionListItem { name : "akonadi_agentinstance_create" }
    ]
  }

  ActionList {
    category : "account"
    name : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_synchronize" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    FakeAction { name : "purge_completed_todos" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_move_to_menu" }
    ActionListItem { name : "akonadi_collection_copy_to_menu" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_view_as_favorite" }
    FakeAction { name : "start_maintenance" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "Folders" )
    ActionListItem { name : "akonadi_collection_sync" }
    FakeAction { name : "purge_completed_todos" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_view_as_favorite" }
    FakeAction { name : "change_folder_selection" }
    FakeAction { name : "start_maintenance" }
  }

  ActionList {
    category : "todo_viewer"
    name : KDE.i18n( "ToDo" )
    FakeAction { name : "new_sub_todo" }
    FakeAction { name : "make_sub_todo_independent" }
    FakeAction { name : "make_all_sub_todos_independent" }
  }

  ActionList {
    category : "todo_viewer"
    name : KDE.i18n( "Attachments" )
    FakeAction { name : "save_all" }
  }

  ActionList {
    category : "todo_viewer"
    name : KDE.i18n( "Edit" )
    FakeAction { name : "edit_todo" }
    ActionListItem { name : "akonadi_item_copy_to_menu" }
    ActionListItem { name : "akonadi_item_move_to_menu" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : KDE.i18n( "KTasks" )
    type : "tasks"
  }
}
