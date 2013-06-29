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

import QtQuick 1.1 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5
// FIXME: how do I do this correctly??
import "../mobileui/ScreenFunctions.js" as Screen

ActionMenuContainer {

  menuStyle : true

  actionItemHeight: Screen.partition( height, 6 ) - actionItemSpacing
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
      reactsOnLongPressed : true
    }
    ScriptActionItem { name : "to_selection_screen"; title: KDE.i18n( "Select Multiple Folders" ) }
    ActionListItem { name : "akonadi_empty_all_trash" }
    ActionListItem { name : "import_emails" }
  }

  FavoriteManager{
    model : favoritesList
    actionItemHeight: parent.actionItemHeight
  }

  ActionList {
    category : "account"
    name : "account_menu"
    text : KDE.i18n( "Account" )
    ActionListItem { name : "akonadi_resource_synchronize" }
    ActionListItem { name : "akonadi_manage_local_subscriptions" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "export_account_emails" }
  }

  ActionList {
    category : "single_folder"
    name : "single_folder_folder_menu"
    text : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as_read" }
    ActionListItem { name : "move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
    ActionListItem { name : "export_selected_emails" }
    ActionListItem { name : "apply_filters" }
    ScriptActionItem { name : "edit_acls"; title : KDE.i18n( "Edit ACLs" ); visible: aclEditor.collectionHasAcls }
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
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
    ActionListItem { name : "prefer_html_to_plain" }
    ActionListItem { name : "load_external_ref" }
    ActionListItem { name : "messagelist_change_settings" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_folder_menu"
    text : KDE.i18n( "Folders" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as_read" }
    ActionListItem { name : "akonadi_move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
    ActionListItem { name : "export_selected_emails" }
    ActionListItem { name : "apply_filters" }
  }

  ActionList {
    category : "multiple_folder"
    name : "multi_folder_view_menu"
    text : KDE.i18n( "View" )
    ScriptActionItem { name : "add_as_favorite"; title : KDE.i18n( "Add View As Favorite" ); visible: !guiStateManager.inSearchResultScreenState }
    ScriptActionItem { name : "to_selection_screen"; title: KDE.i18n( "Select Folders" ) }
    ScriptActionItem { name : "start_maintenance"; title : KDE.i18n( "Switch To Editing Mode" ) }
    ActionListItem { name : "prefer_html_to_plain" }
    ActionListItem { name : "load_external_ref" }
    ActionListItem { name : "messagelist_change_settings" }
  }

  ActionList {
    category: "mail_viewer"
    name : "email_menu"
    text : KDE.i18n( "Email" )

    ActionListItem {
      name : "message_reply"
      onPressAndHold: {
          pageStack.push(Qt.createComponent("ReplyOptionsPage.qml") )
          actionPanel.collapse()
      }
      reactsOnLongPressed : true
    }

    ActionListItem {
      name : "message_forward"
      onPressAndHold: {
          pageStack.push(Qt.createComponent("ForwardOptionsPage.qml") )
          actionPanel.collapse()
      }
      reactsOnLongPressed : true
    }

    ScriptActionItem { name : "mark_as_dialog"; title : KDE.i18n( "Mark Email As" ) }
    ActionListItem { name : "message_send_again"; visible : application.collectionIsSentMail }
    ActionListItem { name : "create_todo_reminder" }
    ActionListItem { name : "create_event" }
    ActionListItem { name : "message_find_in" }
    ActionListItem { name : "message_save_as" }
    ActionListItem { name : "apply_filters" }
  }


  ActionList {
    category: "mail_viewer"
    name : "mail_viewer_view"
    text : KDE.i18n( "View" )
    ActionListItem { name : "message_fixed_font" }
    ScriptActionItem { name : "copy_all_to_clipboard"; title : KDE.i18n( "Copy Email To Clipboard" ) }
    ActionListItem { name : "show_message_source" }
    ActionListItem { name : "change_message_encoding" }
    ActionListItem { name : "show_extended_headers" }
    ActionListItem { name : "prefer_html_to_plain_viewer" }
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
    text : KDE.i18n( "Mail" )
    type : "mail"

    addNewActionName: "add_new_mail"
    addNewActionReactsOnLongPressed: true

    searchActionTitle: KDE.i18n( "Search For Emails" )
    configureActionTitle: KDE.i18n( "Configure Mail" )

    onLongPressed : {
      if ( actionName == "add_new_mail" ) {
          pageStack.push(Qt.createComponent("NewMailPage.qml") )
          actionPanel.collapse()
      }
    }

    ActionListItem { name : "tools_edit_vacation" }
  }
}
