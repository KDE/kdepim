/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#include <qcolor.h>
#include <qstring.h>

/**
 * Namespace for the names of the config groups and keys used.
 * @author Rikkus
 */
namespace EmpathConfig
{

const QString GROUP_GENERAL     = "General";
const QString GROUP_FOLDERS     = "Folders";
const QString GROUP_MAILBOX     = "Mailbox_";
const QString GROUP_FILTER      = "Filter_";
const QString GROUP_ACTION      = "Action_";
const QString GROUP_EXPR        = "Expr_";
const QString GROUP_DISPLAY     = "Display";
const QString GROUP_COMPOSE     = "Compose";
const QString GROUP_IDENTITY    = "Identity";
const QString GROUP_SENDING     = "Sending";
const QString GROUP_ACCOUNT     = "Account";

// Keys

// GENERAL
const QString GEN_MAILBOX_LIST  = "MailboxList";
const QString GEN_SAVE_POLICY   = "SavePolicy";

// UI
const QString UI_THREAD             = "ThreadMessages";
const QString UI_SHOW_HEADERS       = "ShowHeaders";
const QString UI_MAIN_WIN_X         = "MainWindowXSize";
const QString UI_MAIN_WIN_Y         = "MainWindowYSize";
const QString UI_MAIN_W_V           = "MainWidgetVSepPos";
const QString UI_MAIN_W_H           = "MainWidgetHSepPos";
const QString UI_SORT_COLUMN        = "MessageListSortColumn";
const QString UI_SORT_ASCENDING     = "MessageListSortAscending";
const QString UI_MAIN_WIN_TOOL      = "MainWindowToolbarPos";
const QString UI_COMPOSE_WIN_TOOL   = "ComposeWindowToolbarPos";
const QString UI_TIP_AT_START       = "TipOfTheDayAtStartup";
const QString UI_MSG_LIST_SIZE      = "MessageListSizeOfColumn";
const QString UI_MSG_LIST_POS       = "MessageListPositionOfColumn";
const QString UI_FIXED_FONT         = "FixedFont";
const QString UI_UNDERLINE_LINKS    = "UnderlineLinks";
const QString UI_QUOTE_ONE          = "QuoteColourOne";
const QString UI_QUOTE_TWO          = "QuoteColourTwo";
const QString UI_LINK               = "LinkColour";
const QString UI_VLINK              = "VisitedLinkColour";
const QString UI_STAR_PASSWORD      = "StarPassword";
const QString UI_FOLDERS_OPEN       = "FolderListItemsOpen";
const QString UI_MARK_READ          = "MarkAsRead";
const QString UI_MARK_TIME          = "MarkAsReadAfterTime";

// MAILBOX
const QString M_TYPE        = "Type";
const QString M_NAME        = "Name";
const QString M_PATH        = "Path";
const QString M_FOLDER_LIST = "FolderList";
const QString M_USERNAME    = "Username";
const QString M_PASSWORD    = "Password";
const QString M_ADDRESS     = "Address";
const QString M_PORT        = "Port";
const QString M_CHECK       = "CheckMailAtIntervals";
const QString M_CHECK_INT   = "CheckMailInterval";
// MAILDIR
const QString M_INTERNAL_FILE = "EmpathMaildir";
// NETWORK
const QString M_LOGGING         = "LoggingPolicy";
const QString M_LOG_PATH        = "LogFilePath";
const QString M_LOG_DISPOSAL    = "LogFileDisposal";
const QString M_MAX_LOG_SIZE    = "LogFileMaxSize";

// FOLDERS
const QString FOLDER_INBOX  = "Inbox";
const QString FOLDER_DRAFTS = "Drafts";
const QString FOLDER_OUTBOX = "Outbox";
const QString FOLDER_SENT   = "Sent";
const QString FOLDER_TRASH  = "Trash";

// COMPOSING
const QString C_EXTRA_HEADERS       = "ExtraHeaders";
const QString C_PHRASE_REPLY_SENDER = "PhraseReplySender";
const QString C_PHRASE_REPLY_ALL    = "PhraseReplyAll";
const QString C_PHRASE_FORWARD      = "PhraseForward";
const QString C_AUTO_QUOTE          = "AutoQuoteOnReply";
const QString C_USE_EXT_EDIT        = "UseExternalEditor";
const QString C_EXT_EDIT            = "ExternalEditor";
const QString C_CC_OTHER            = "CopyOther";
const QString C_CC_OTHER_ADDRESS    = "CopyOtherAddress";
const QString C_SIG_PATH            = "Signature";
const QString C_ADD_SIG             = "AddSignature";
const QString C_ADD_DIG_SIG         = "AddDigitalSignature";
const QString C_SEND_POLICY         = "SendPolicy";
const QString C_WRAP_LINES          = "WrapLongLines";
const QString C_WRAP_COLUMN         = "WrapColumn";

// SENDING
const QString S_ENCRYPT         = "Encrypt";
const QString S_TYPE            = "OutgoingServerType";
const QString S_SENDMAIL        = "SendmailLocation";
const QString S_QMAIL           = "QmailLocation";
const QString S_SMTP            = "SMTPServerLocation";
const QString S_SMTP_PORT       = "SMTPServerPort";
const QString S_CONFIRM_DELIVER = "ConfirmDelivery";
const QString S_CONFIRM_READ    = "ConfirmReading";

// FILTERS
const QString F_LIST            = "List";
const QString F_EXPRS           = "MatchExpressions";
const QString F_FOLDER          = "SourceFolder";
const QString F_PRIORITY        = "Priority";
const QString F_ACTION_TYPE     = "Type";
const QString F_ACTION_FOLDER   = "Folder";
const QString F_ACTION_ADDRESS  = "Address";
const QString F_MATCH_TYPE      = "Type";
const QString F_MATCH_SIZE      = "Size";
const QString F_MATCH_EXPR      = "Expr";
const QString F_MATCH_HEADER    = "HeaderExpr";

// Defaults

const QString   DFLT_REPLY          = "On %1, you wrote:";
const QString   DFLT_REPLY_ALL      = "On %1, %2 wrote:";
const QString   DFLT_FORWARD        = "Forwarded message from %1";
const QString   DFLT_HEADERS        = "From,Date,Subject";

const QColor    DFLT_Q_1            = Qt::darkBlue;
const QColor    DFLT_Q_2            = Qt::darkCyan;
const QColor    DFLT_LINK           = Qt::blue;
const QColor    DFLT_VLINK          = Qt::darkRed;

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
