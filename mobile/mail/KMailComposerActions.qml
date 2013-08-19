/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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
import "../mobileui/ScreenFunctions.js" as Screen

ActionMenuContainer {
  id: root
  menuStyle : true

  actionItemHeight: Screen.partition( height, 6 ) - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "composer"
    name : "message_menu"
    text : KDE.i18n( "Message" )
    ActionListItem { name: "send_later" }
    ActionListItem { name: "save_in_drafts" }
    ActionListItem { name: "save_as_template" }
  }

  ActionList {
    category : "composer"
    name : "edit_menu"
    text : KDE.i18n( "Edit" )
    ActionListItem { name : "composer_search" }
    ActionListItem { name : "composer_search_next" }
    ActionListItem { name : "composer_replace" }
    ActionListItem { name : "composer_clean_spaces" }
    ActionListItem { name : "composer_add_quote_char" }
    ActionListItem { name : "composer_remove_quote_char" }
    ActionListItem { name : "composer_spell_check" }
  }

  ActionList {
    category : "composer"
    name : "options_menu"
    text : KDE.i18n( "Options" )
    ActionListItem { name : "options_mark_as_urgent" }
    ActionListItem { name : "options_request_mdn" }
    ActionListItem { name : "options_wordwrap" }
    ActionListItem { name : "options_fixedfont" }
    ActionListItem { name : "attach_public_key" }
    ActionListItem { name : "options_set_cryptoformat" }
  }

  ActionList {
    category : "composer"
    name : "signature_menu"
    text : KDE.i18n( "Signature" )
    ActionListItem { name : "composer_append_signature" }
    ActionListItem { name : "composer_prepend_signature" }
    ActionListItem { name : "composer_insert_signature" }
  }

  ActionList {
    category : "composer"
    name : "security_menu"
    text : KDE.i18n( "Security" )
    ActionListItem { name : "sign_email" }
    ActionListItem { name : "encrypt_email" }
  }

  SnippetsList {
    category : "composer"
    name : "snippets_menu"
    text : KDE.i18n( "Snippets" )
    actionItemHeight: root.actionItemHeight
  }

  ActionList {
    category : "composer"
    name : "composer_menu"
    text : KDE.i18n( "Composer" )
    ScriptActionItem { name : "composer_configure_identity"; title: KDE.i18n( "Configure Identity" ) }
    ScriptActionItem { name : "composer_configure_transport"; title: KDE.i18n( "Configure Transport" ) }
    ScriptActionItem { name : "composer_close"; title: KDE.i18n( "Close Composer" ) }
    ActionListItem { name : "wm_task_switch" }
  }

}
