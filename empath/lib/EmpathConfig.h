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

static const char * GROUP_GENERAL     = "General";
static const char * GROUP_FOLDERS     = "Folders";
static const char * GROUP_MAILBOX     = "Mailbox_";
static const char * GROUP_FILTER      = "Filter_";
static const char * GROUP_ACTION      = "Action_";
static const char * GROUP_EXPR        = "Expr_";
static const char * GROUP_DISPLAY     = "Display";
static const char * GROUP_COMPOSE     = "Compose";
static const char * GROUP_IDENTITY    = "Identity";
static const char * GROUP_SENDING     = "Sending";
static const char * GROUP_ACCOUNT     = "Account";

// Keys

// GENERAL
static const char * GEN_MAILBOX_LIST  = "MailboxList";
static const char * GEN_SAVE_POLICY   = "SavePolicy";

// UI
static const char * UI_THREAD             = "ThreadMessages";
static const char * UI_SHOW_HEADERS       = "ShowHeaders";
static const char * UI_MAIN_WIN_X         = "MainWindowXSize";
static const char * UI_MAIN_WIN_Y         = "MainWindowYSize";
static const char * UI_MAIN_W_V           = "MainWidgetVSepPos";
static const char * UI_MAIN_W_H           = "MainWidgetHSepPos";
static const char * UI_SORT_COLUMN        = "MessageListSortColumn";
static const char * UI_SORT_ASCENDING     = "MessageListSortAscending";
static const char * UI_MAIN_WIN_TOOL      = "MainWindowToolbarPos";
static const char * UI_COMPOSE_WIN_TOOL   = "ComposeWindowToolbarPos";
static const char * UI_TIP_AT_START       = "TipOfTheDayAtStartup";
static const char * UI_MSG_LIST_SIZE      = "MessageListSizeOfColumn";
static const char * UI_MSG_LIST_POS       = "MessageListPositionOfColumn";
static const char * UI_FIXED_FONT         = "FixedFont";
static const char * UI_UNDERLINE_LINKS    = "UnderlineLinks";
static const char * UI_QUOTE_ONE          = "QuoteColourOne";
static const char * UI_QUOTE_TWO          = "QuoteColourTwo";
static const char * UI_LINK               = "LinkColour";
static const char * UI_NEW                = "NewMessageColour";
static const char * UI_STAR_PASSWORD      = "StarPassword";
static const char * UI_FOLDERS_OPEN       = "FolderListItemsOpen";
static const char * UI_MARK_READ          = "MarkAsRead";
static const char * UI_MARK_TIME          = "MarkAsReadAfterTime";
static const char * UI_LAST_SHOWN_FOLDER  = "LastShownFolder";

// MAILBOX
static const char * M_TYPE        = "Type";
static const char * M_NAME        = "Name";
static const char * M_PATH        = "Path";
static const char * M_FOLDER_LIST = "FolderList";
static const char * M_USERNAME    = "Username";
static const char * M_PASSWORD    = "Password";
static const char * M_ADDRESS     = "Address";
static const char * M_PORT        = "Port";
static const char * M_CHECK       = "CheckMailAtIntervals";
static const char * M_CHECK_INT   = "CheckMailInterval";
// MAILDIR
static const char * M_INTERNAL_FILE = "EmpathMaildir";
// NETWORK
static const char * M_LOGGING         = "LoggingPolicy";
static const char * M_LOG_PATH        = "LogFilePath";
static const char * M_LOG_DISPOSAL    = "LogFileDisposal";
static const char * M_MAX_LOG_SIZE    = "LogFileMaxSize";

// FOLDERS
static const char * FOLDER_INBOX  = "Inbox";
static const char * FOLDER_DRAFTS = "Drafts";
static const char * FOLDER_OUTBOX = "Outbox";
static const char * FOLDER_SENT   = "Sent";
static const char * FOLDER_TRASH  = "Trash";

// COMPOSING
static const char * C_EXTRA_HEADERS       = "ExtraHeaders";
static const char * C_PHRASE_REPLY_SENDER = "PhraseReplySender";
static const char * C_PHRASE_REPLY_ALL    = "PhraseReplyAll";
static const char * C_PHRASE_FORWARD      = "PhraseForward";
static const char * C_AUTO_QUOTE          = "AutoQuoteOnReply";
static const char * C_USE_EXT_EDIT        = "UseExternalEditor";
static const char * C_EXT_EDIT            = "ExternalEditor";
static const char * C_CC_OTHER            = "CopyOther";
static const char * C_CC_OTHER_ADDRESS    = "CopyOtherAddress";
static const char * C_SIG_PATH            = "Signature";
static const char * C_ADD_SIG             = "AddSignature";
static const char * C_ADD_DIG_SIG         = "AddDigitalSignature";
static const char * C_SEND_POLICY         = "SendPolicy";
static const char * C_WRAP_LINES          = "WrapLongLines";
static const char * C_WRAP_COLUMN         = "WrapColumn";

// SENDING
static const char * S_ENCRYPT         = "Encrypt";
static const char * S_TYPE            = "OutgoingServerType";
static const char * S_SENDMAIL        = "SendmailLocation";
static const char * S_QMAIL           = "QmailLocation";
static const char * S_SMTP            = "SMTPServerLocation";
static const char * S_SMTP_PORT       = "SMTPServerPort";
static const char * S_CONFIRM_DELIVER = "ConfirmDelivery";
static const char * S_CONFIRM_READ    = "ConfirmReading";

// FILTERS
static const char * F_LIST            = "List";
static const char * F_EXPRS           = "MatchExpressions";
static const char * F_FOLDER          = "SourceFolder";
static const char * F_PRIORITY        = "Priority";
static const char * F_ACTION_TYPE     = "Type";
static const char * F_ACTION_FOLDER   = "Folder";
static const char * F_ACTION_ADDRESS  = "Address";
static const char * F_MATCH_TYPE      = "Type";
static const char * F_MATCH_SIZE      = "Size";
static const char * F_MATCH_EXPR      = "Expr";
static const char * F_MATCH_HEADER    = "HeaderExpr";

// Defaults

static const char *   DFLT_REPLY          = "On %1, you wrote:";
static const char *   DFLT_REPLY_ALL      = "On %1, %2 wrote:";
static const char *   DFLT_FORWARD        = "Forwarded message from %1";
static const char *   DFLT_HEADERS        = "From,Date,Subject";

static QColor *  DFLT_Q_1;
static QColor *  DFLT_Q_2;
static QColor *  DFLT_LINK;
static QColor *  DFLT_NEW;

static const bool      DFLT_UNDER_LINKS    = true;
static const bool      DFLT_AUTO_QUOTE     = true;
static const bool      DFLT_SIGN           = true;
static const bool      DFLT_DIG_SIGN       = false;
static const bool      DFLT_CC_ME          = false;
static const bool      DFLT_CC_OTHER       = false;
static const bool      DFLT_SEND_NOW       = true;
static const bool      DFLT_CHECK_NEW      = true;
static const bool      DFLT_THREAD         = true;
static const bool      DFLT_SORT_ASCENDING = true;
static const bool      DFLT_MARK           = true;

static const unsigned  DFLT_MARK_TIMER     = 2;
static const unsigned  DFLT_SORT_COL       = 2;
static const unsigned  DFLT_WRAP           = 76;
static const unsigned  DFLT_CHECK_INT      = 10;
};

#endif
 
// vim:ts=4:sw=4:tw=78
