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

#define GROUP_GENERAL     "General"
#define GROUP_FOLDERS     "Folders"
#define GROUP_MAILBOX     "Mailbox_"
#define GROUP_FILTER      "Filter_"
#define GROUP_ACTION      "Action_"
#define GROUP_EXPR        "Expr_"
#define GROUP_DISPLAY     "Display"
#define GROUP_COMPOSE     "Compose"
#define GROUP_IDENTITY    "Identity"
#define GROUP_SENDING     "Sending"
#define GROUP_ACCOUNT     "Account"

// Keys

// GENERAL
#define GEN_MAILBOX_LIST  "MailboxList"
#define GEN_SAVE_POLICY   "SavePolicy"

// UI
#define UI_THREAD             "ThreadMessages"
#define UI_SHOW_HEADERS       "ShowHeaders"
#define UI_MAIN_WIN_X         "MainWindowXSize"
#define UI_MAIN_WIN_Y         "MainWindowYSize"
#define UI_MAIN_W_V           "MainWidgetVSepPos"
#define UI_MAIN_W_H           "MainWidgetHSepPos"
#define UI_SORT_COLUMN        "MessageListSortColumn"
#define UI_SORT_ASCENDING     "MessageListSortAscending"
#define UI_MAIN_WIN_TOOL      "MainWindowToolbarPos"
#define UI_COMPOSE_WIN_TOOL   "ComposeWindowToolbarPos"
#define UI_TIP_AT_START       "TipOfTheDayAtStartup"
#define UI_MSG_LIST_SIZE      "MessageListSizeOfColumn"
#define UI_MSG_LIST_POS       "MessageListPositionOfColumn"
#define UI_FIXED_FONT         "FixedFont"
#define UI_UNDERLINE_LINKS    "UnderlineLinks"
#define UI_QUOTE_ONE          "QuoteColourOne"
#define UI_QUOTE_TWO          "QuoteColourTwo"
#define UI_LINK               "LinkColour"
#define UI_NEW                "NewMessageColour"
#define UI_STAR_PASSWORD      "StarPassword"
#define UI_FOLDERS_OPEN       "FolderListItemsOpen"
#define UI_MARK_READ          "MarkAsRead"
#define UI_MARK_TIME          "MarkAsReadAfterTime"
#define UI_LAST_SHOWN_FOLDER  "LastShownFolder"

// MAILBOX
#define M_TYPE        "Type"
#define M_NAME        "Name"
#define M_PATH        "Path"
#define M_FOLDER_LIST "FolderList"
#define M_USERNAME    "Username"
#define M_PASSWORD    "Password"
#define M_ADDRESS     "Address"
#define M_PORT        "Port"
#define M_CHECK       "CheckMailAtIntervals"
#define M_CHECK_INT   "CheckMailInterval"
// MAILDIR
#define M_INTERNAL_FILE "EmpathMaildir"
// NETWORK
#define M_LOGGING         "LoggingPolicy"
#define M_LOG_PATH        "LogFilePath"
#define M_LOG_DISPOSAL    "LogFileDisposal"
#define M_MAX_LOG_SIZE    "LogFileMaxSize"

// FOLDERS
#define FOLDER_INBOX  "Inbox"
#define FOLDER_DRAFTS "Drafts"
#define FOLDER_OUTBOX "Outbox"
#define FOLDER_SENT   "Sent"
#define FOLDER_TRASH  "Trash"

// COMPOSING
#define C_EXTRA_HEADERS       "ExtraHeaders"
#define C_PHRASE_REPLY_SENDER "PhraseReplySender"
#define C_PHRASE_REPLY_ALL    "PhraseReplyAll"
#define C_PHRASE_FORWARD      "PhraseForward"
#define C_AUTO_QUOTE          "AutoQuoteOnReply"
#define C_USE_EXT_EDIT        "UseExternalEditor"
#define C_EXT_EDIT            "ExternalEditor"
#define C_CC_OTHER            "CopyOther"
#define C_CC_OTHER_ADDRESS    "CopyOtherAddress"
#define C_SIG_PATH            "Signature"
#define C_ADD_SIG             "AddSignature"
#define C_ADD_DIG_SIG         "AddDigitalSignature"
#define C_SEND_POLICY         "SendPolicy"
#define C_WRAP_LINES          "WrapLongLines"
#define C_WRAP_COLUMN         "WrapColumn"

// SENDING
#define S_ENCRYPT         "Encrypt"
#define S_TYPE            "OutgoingServerType"
#define S_SENDMAIL        "SendmailLocation"
#define S_QMAIL           "QmailLocation"
#define S_SMTP            "SMTPServerLocation"
#define S_SMTP_PORT       "SMTPServerPort"
#define S_CONFIRM_DELIVER "ConfirmDelivery"
#define S_CONFIRM_READ    "ConfirmReading"

// FILTERS
#define F_LIST            "List"
#define F_EXPRS           "MatchExpressions"
#define F_FOLDER          "SourceFolder"
#define F_PRIORITY        "Priority"
#define F_ACTION_TYPE     "Type"
#define F_ACTION_FOLDER   "Folder"
#define F_ACTION_ADDRESS  "Address"
#define F_MATCH_TYPE      "Type"
#define F_MATCH_SIZE      "Size"
#define F_MATCH_EXPR      "Expr"
#define F_MATCH_HEADER    "HeaderExpr"

// Defaults

#define DFLT_REPLY          "On %1, you wrote:"
#define DFLT_REPLY_ALL      "On %1, %2 wrote:"
#define DFLT_FORWARD        "Forwarded message from %1"
#define DFLT_HEADERS        "From,Date,Subject"

#define DFLT_UNDER_LINKS    true
#define DFLT_AUTO_QUOTE     true
#define DFLT_SIGN           true
#define DFLT_DIG_SIGN       false
#define DFLT_CC_ME          false
#define DFLT_CC_OTHER       false
#define DFLT_SEND_NOW       true
#define DFLT_CHECK_NEW      true
#define DFLT_THREAD         true
#define DFLT_SORT_ASCENDING true
#define DFLT_MARK           true

#define DFLT_MARK_TIMER     2
#define DFLT_SORT_COL       2
#define DFLT_WRAP           76
#define DFLT_CHECK_INT      10
};

#endif
 
// vim:ts=4:sw=4:tw=78
