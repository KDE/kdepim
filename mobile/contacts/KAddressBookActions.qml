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
    name : "home_menu"
    text : KDE.i18n( "Home" )
    ActionListItem { name : "synchronize_all_items" }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Select multiple folders" ) }
  }

  FavoriteManager{ model : favoritesList }

  AgentInstanceList {
    category : "home"
    name : "accounts_list"
    text : KDE.i18n( "Accounts" )

    model : agentInstanceList

    customActions : [
      ActionListItem { name : "akonadi_agentinstance_configure" },
      ActionListItem { name : "akonadi_agentinstance_delete" },
      ActionListItem { name : "akonadi_agentinstance_create" }
    ]
  }

  ActionList {
    category : "account"
    name : "account_menu"
    text : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_synchronize" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_folder_menu"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_move_to_menu" }
    ActionListItem { name : "akonadi_collection_copy_to_menu" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add as Favorite" ) }
    FakeAction { name : "start_maintenance" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_folder_menu"
    text : KDE.i18n( "Folders" )
    ActionListItem { name : "akonadi_collection_sync" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add as Favorite" ) }
    ScriptActionItem { name : "to_selection_screen"; title : KDE.i18n( "Back to folder selection" ) }
    FakeAction { name : "start_maintenance" }
  }

  ActionList {
    category : "contact_viewer"
    name : "contact_viewer_contact_menu"
    text : KDE.i18n( "Contact" )
  }

  ActionList {
    category : "contact_viewer"
    name : "contact_viewer_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_contact_item_edit" }
    ActionListItem { name : "akonadi_item_copy_to_menu" }
    ActionListItem { name : "akonadi_item_move_to_menu" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ApplicationGeneralActions {
    name : "application_menu"
    category : "standard"
    text : KDE.i18n( "KAddressBook" )
    type : "contacts"
  }
}
