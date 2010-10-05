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

  menuStyle : true

  actionItemHeight : height / 6 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : "home_menu"
    text : KDE.i18n( "Home" )
    ActionListItem { name : "synchronize_all_items" }
    ActionListItem {
      name : "send_queued_emails" 
      onPressAndHold: {
          application.getAction("send_queued_emails_via", "").trigger();
          actionPanel.collapse()
      }
    }
    ScriptActionItem { name : "to_selection_screen"; title: KDE.i18n( "Select multiple folders" ) }
    ActionListItem { name : "import_emails" }
    ActionListItem { name : "export_emails" }
    ActionListItem { name : "akonadi_empty_all_trash" }
  }

  FavoriteManager{
    model : favoritesList
  }

  AgentInstanceList {
    category : "home"
    name : "accounts_list"
    text : KDE.i18n( "Accounts" )

    model : agentInstanceList
  }

  ActionListItem {
    category : "home"
    name : "kmail_mobile_identities"
  }

  ActionList {
    category : "account"
    name : "account_menu"
    text : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_synchronize" }
    ActionListItem { name : "akonadi_manage_local_subscriptions" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_folder_menu"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as_read" }
    ActionListItem { name : "move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
    ActionListItem { name : "show_expire_properties" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_move_to_dialog" }
    ActionListItem { name : "akonadi_collection_copy_to_dialog" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add as Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Start Maintenance" ) }
    ActionListItem { name : "prefer_html_to_plain" }
    ActionListItem { name : "load_external_ref" }
  }

  ActionListItem {
    category: "single_folder"
    name : "write_new_email"
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_folder_menu"
    text : KDE.i18n( "Folders" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as_read" }
    ActionListItem { name : "akonadi_move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add as Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Start Maintenance" ) }
    ActionListItem { name : "prefer_html_to_plain" }
    ActionListItem { name : "load_external_ref" }
  }

  ActionList {
    category: "mail_viewer"
    name : "email_menu"
    text : KDE.i18n( "Email" )

    ActionListItem {
      name : "message_reply"
      onPressAndHold: {
          replyOptionsPage.visible = true
          actionPanel.collapse()
      }      
    }
  
    ActionListItem {
      name : "message_forward"
      onPressAndHold: {
          forwardOptionsPage.visible = true
          actionPanel.collapse()
      }
    }

    ScriptActionItem { name : "mark_as_dialog"; title : KDE.i18n( "Mark As..." ) }
    ActionListItem { name : "message_send_again" }
    ActionListItem { name : "create_todo_reminder" }
    ActionListItem { name : "message_find_in" }
    ActionListItem { name : "message_save_as" }
  }


  ActionList {
    category: "mail_viewer"
    name : "mail_viewer_attachments"
    text : KDE.i18n( "Attachments" )
    ScriptActionItem { name : "attachment_save_all"; title : KDE.i18n( "Save All Attachments" ) }
   // ActionListItem { name : "attachment_save_all" }
  }

  ActionList {
    category: "mail_viewer"
    name : "mail_viewer_edit"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "message_edit" }
    ActionListItem { name : "akonadi_item_copy_to_dialog" }
    ActionListItem { name : "akonadi_item_move_to_dialog" }
    ActionListItem { name : "akonadi_move_to_trash" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : "application_menu"
    text : KDE.i18n( "Application" )
    type : "mail"

  }
}
