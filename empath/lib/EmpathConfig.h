/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EMPATHCONFIG_H
#define EMPATHCONFIG_H

class QColor;

/**
 * Namespace for the names of the config groups and keys used.
 * @author Rikkus
 */
namespace EmpathConfig
{

const char * GROUP_GENERAL     = "General";
const char * GROUP_FOLDERS     = "Folders";
const char * GROUP_MAILBOX     = "Mailbox_";
const char * GROUP_FILTER      = "Filter_";
const char * GROUP_ACTION      = "Action_";
const char * GROUP_EXPR        = "Expr_";
const char * GROUP_DISPLAY     = "Display";
const char * GROUP_COMPOSE     = "Compose";
const char * GROUP_IDENTITY    = "Identity";
const char * GROUP_SENDING     = "Sending";
const char * GROUP_ACCOUNT     = "Account";

// Keys

// GENERAL
const char * GEN_MAILBOX_LIST  = "MailboxList";
const char * GEN_SAVE_POLICY   = "SavePolicy";

// UI
const char * UI_THREAD             = "ThreadMessages";
const char * UI_SHOW_HEADERS       = "ShowHeaders";
const char * UI_MAIN_WIN_X         = "MainWindowXSize";
const char * UI_MAIN_WIN_Y         = "MainWindowYSize";
const char * UI_MAIN_W_V           = "MainWidgetVSepPos";
const char * UI_MAIN_W_H           = "MainWidgetHSepPos";
const char * UI_SORT_COLUMN        = "MessageListSortColumn";
const char * UI_SORT_ASCENDING     = "MessageListSortAscending";
const char * UI_MAIN_WIN_TOOL      = "MainWindowToolbarPos";
const char * UI_COMPOSE_WIN_TOOL   = "ComposeWindowToolbarPos";
const char * UI_TIP_AT_START       = "TipOfTheDayAtStartup";
const char * UI_MSG_LIST_SIZE      = "MessageListSizeOfColumn";
const char * UI_MSG_LIST_POS       = "MessageListPositionOfColumn";
const char * UI_FIXED_FONT         = "FixedFont";
const char * UI_UNDERLINE_LINKS    = "UnderlineLinks";
const char * UI_QUOTE_ONE          = "QuoteColourOne";
const char * UI_QUOTE_TWO          = "QuoteColourTwo";
const char * UI_LINK               = "LinkColour";
const char * UI_NEW                = "NewMessageColour";
const char * UI_STAR_PASSWORD      = "StarPassword";
const char * UI_FOLDERS_OPEN       = "FolderListItemsOpen";
const char * UI_MARK_READ          = "MarkAsRead";
const char * UI_MARK_TIME          = "MarkAsReadAfterTime";
const char * UI_LAST_SHOWN_FOLDER  = "LastShownFolder";

// MAILBOX
const char * M_TYPE        = "Type";
const char * M_NAME        = "Name";
const char * M_PATH        = "Path";
const char * M_FOLDER_LIST = "FolderList";
const char * M_USERNAME    = "Username";
const char * M_PASSWORD    = "Password";
const char * M_ADDRESS     = "Address";
const char * M_PORT        = "Port";
const char * M_CHECK       = "CheckMailAtIntervals";
const char * M_CHECK_INT   = "CheckMailInterval";
// MAILDIR
const char * M_INTERNAL_FILE = "EmpathMaildir";
// NETWORK
const char * M_LOGGING         = "LoggingPolicy";
const char * M_LOG_PATH        = "LogFilePath";
const char * M_LOG_DISPOSAL    = "LogFileDisposal";
const char * M_MAX_LOG_SIZE    = "LogFileMaxSize";

// FOLDERS
const char * FOLDER_INBOX  = "Inbox";
const char * FOLDER_DRAFTS = "Drafts";
const char * FOLDER_OUTBOX = "Outbox";
const char * FOLDER_SENT   = "Sent";
const char * FOLDER_TRASH  = "Trash";

// COMPOSING
const char * C_EXTRA_HEADERS       = "ExtraHeaders";
const char * C_PHRASE_REPLY_SENDER = "PhraseReplySender";
const char * C_PHRASE_REPLY_ALL    = "PhraseReplyAll";
const char * C_PHRASE_FORWARD      = "PhraseForward";
const char * C_AUTO_QUOTE          = "AutoQuoteOnReply";
const char * C_USE_EXT_EDIT        = "UseExternalEditor";
const char * C_EXT_EDIT            = "ExternalEditor";
const char * C_CC_OTHER            = "CopyOther";
const char * C_CC_OTHER_ADDRESS    = "CopyOtherAddress";
const char * C_SIG_PATH            = "Signature";
const char * C_ADD_SIG             = "AddSignature";
const char * C_ADD_DIG_SIG         = "AddDigitalSignature";
const char * C_SEND_POLICY         = "SendPolicy";
const char * C_WRAP_LINES          = "WrapLongLines";
const char * C_WRAP_COLUMN         = "WrapColumn";

// SENDING
const char * S_ENCRYPT         = "Encrypt";
const char * S_TYPE            = "OutgoingServerType";
const char * S_SENDMAIL        = "SendmailLocation";
const char * S_QMAIL           = "QmailLocation";
const char * S_SMTP            = "SMTPServerLocation";
const char * S_SMTP_PORT       = "SMTPServerPort";
const char * S_CONFIRM_DELIVER = "ConfirmDelivery";
const char * S_CONFIRM_READ    = "ConfirmReading";

// FILTERS
const char * F_LIST            = "List";
const char * F_EXPRS           = "MatchExpressions";
const char * F_FOLDER          = "SourceFolder";
const char * F_PRIORITY        = "Priority";
const char * F_ACTION_TYPE     = "Type";
const char * F_ACTION_FOLDER   = "Folder";
const char * F_ACTION_ADDRESS  = "Address";
const char * F_MATCH_TYPE      = "Type";
const char * F_MATCH_SIZE      = "Size";
const char * F_MATCH_EXPR      = "Expr";
const char * F_MATCH_HEADER    = "HeaderExpr";

// Defaults

const char *   DFLT_REPLY          = "On %1, you wrote:";
const char *   DFLT_REPLY_ALL      = "On %1, %2 wrote:";
const char *   DFLT_FORWARD        = "Forwarded message from %1";
const char *   DFLT_HEADERS        = "From,Date,Subject";

const bool      DFLT_UNDER_LINKS    = true;
const bool      DFLT_AUTO_QUOTE     = true;
const bool      DFLT_SIGN           = true;
const bool      DFLT_DIG_SIGN       = false;
const bool      DFLT_CC_ME          = false;
const bool      DFLT_CC_OTHER       = false;
const bool      DFLT_SEND_NOW       = true;
const bool      DFLT_CHECK_NEW      = true;
const bool      DFLT_THREAD         = true;
const bool      DFLT_SORT_ASCENDING = true;
const bool      DFLT_MARK           = true;

const unsigned  DFLT_MARK_TIMER     = 2;
const unsigned  DFLT_SORT_COL       = 2;
const unsigned  DFLT_WRAP           = 76;
const unsigned  DFLT_CHECK_INT      = 10;
};

#endif
 
// vim:ts=4:sw=4:tw=78
