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

#ifdef __GNUG__
# pragma implementation "EmpathConfig.h"
#endif

#include "EmpathConfig.h"

namespace EmpathConfig
{
    
// Groups
const QString GROUP_GENERAL = "General";
const QString GROUP_MAILBOX = "Mailbox_";
const QString GROUP_FILTER = "Filter_";
const QString GROUP_ACTION = "Action_";
const QString GROUP_EXPR = "Expr_";
const QString GROUP_DISPLAY = "Display";
const QString GROUP_COMPOSE = "Compose";
const QString GROUP_IDENTITY = "Identity";
const QString GROUP_SENDING = "Sending";
const QString GROUP_ACCOUNT = "Account";

// Keys
// Kernel
const QString KEY_NUM_MAILBOXES = "NumberOfMailboxes";
const QString KEY_MAILBOX_TYPE = "MailboxType";
const QString KEY_MAILBOX_NAME = "MailboxName";
const QString KEY_MAILBOX_LIST = "MailboxList";
const QString KEY_FOLDER_LIST = "FolderList";
const QString KEY_NAME = "Name";
const QString KEY_EMAIL = "Email";
const QString KEY_REPLY_TO = "ReplyTo";
const QString KEY_ORGANISATION = "Organisation";
const QString KEY_SIG_PATH = "Signature";
const QString KEY_PHRASE_REPLY_SENDER = "PhraseReplySender";
const QString KEY_PHRASE_REPLY_ALL = "PhraseReplyAll";
const QString KEY_PHRASE_FORWARD = "PhraseForward";
const QString KEY_AUTO_QUOTE = "AutoQuoteOnReply";
const QString KEY_ADD_SIG = "AddSignature";
const QString KEY_ADD_DIG_SIG = "AddDigitalSignature";
const QString KEY_WRAP_LINES = "WrapLongLines";
const QString KEY_WRAP_COLUMN = "WrapColumn";
const QString KEY_SEND_POLICY = "SendWhen";
const QString KEY_ACCOUNT_TYPE = "AccountType";
const QString KEY_SERVER_ADDRESS = "ServerAddress";
const QString KEY_SERVER_PORT = "ServerPort";
const QString KEY_SERVER_UNAME = "ServerUsername";
const QString KEY_SERVER_PASSWORD = "ServerPassword";
const QString KEY_CHECK_MAIL = "CheckMailAtIntervals";
const QString KEY_CHECK_MAIL_INTERVAL = "CheckMailInterval";
const QString KEY_INTERNAL_MAIL_FILE = "EmpathMaildir";
const QString KEY_OUTGOING_SERVER_TYPE = "OutgoingServerType";
const QString KEY_SENDMAIL_LOCATION = "SendmailLocation";
const QString KEY_QMAIL_LOCATION = "QmailLocation";
const QString KEY_SMTP_SERVER_LOCATION = "SMTPServerLocation";
const QString KEY_SMTP_SERVER_PORT = "SMTPServerPort";
const QString KEY_POP3_SERVER_ADDRESS = "ServerAddress";
const QString KEY_POP3_SERVER_PORT = "ServerPort";
const QString KEY_POP3_USERNAME = "Username";
const QString KEY_POP3_PASSWORD = "Password";
const QString KEY_POP3_APOP = "UseAPOP";
const QString KEY_POP3_SAVE_POLICY = "SavePolicy";
const QString KEY_POP3_LOGGING_POLICY = "LoggingPolicy";
const QString KEY_POP3_LOG_FILE_PATH = "LogFilePath";
const QString KEY_POP3_LOG_FILE_DISPOSAL_POLICY = "LogFileDisposalPolicy";
const QString KEY_POP3_MAX_LOG_FILE_SIZE = "MaxLogFileSize";
const QString KEY_POP3_MESSAGE_SIZE_THRESHOLD = "MessageSizeThreshold";
const QString KEY_POP3_LARGE_MESSAGE_POLICY = "LargeMessagePolicy";
const QString KEY_POP3_CHECK_FOR_NEW_MAIL = "CheckForNewMail";
const QString KEY_POP3_MAIL_CHECK_INTERVAL = "MailCheckInterval";
const QString KEY_POP3_DELETE_FROM_SERVER = "DeleteFromServer";
const QString KEY_POP3_AUTO_GET_NEW_MAIL = "AutoGetNewMail";
const QString KEY_POP3_SAVE_ALL_ADDRESSES = "SaveAllAddresses";
const QString KEY_POP3_NOTIFY = "Notify";
const QString KEY_POP3_RETRIEVE_IF_HAVE = "RetrieveIfHave";
const QString KEY_FILTER_LIST = "FilterList";
const QString KEY_NUM_MATCH_EXPRS_FOR_FILTER = "NumberOfMatchExpressions";
const QString KEY_MATCH_EXPR_TYPE = "MatchExpressionType";
const QString KEY_MATCH_SIZE = "MatchSize";
const QString KEY_MATCH_EXPR = "MatchExpr";
const QString KEY_MATCH_HEADER = "MatchHeaderExpr";
const QString KEY_FILTER_EVENT_HANDLER_TYPE = "FilterEventHandlerType";
const QString KEY_FILTER_EVENT_HANDLER_FOLDER = "FilterEventHandlerFolder";
const QString KEY_FILTER_EVENT_HANDLER_ADDRESS = "FilterEventHandlerAddress";
const QString KEY_FILTER_FOLDER = "FilterSourceFolder";
const QString KEY_FILTER_PRIORITY = "FilterPriority";
const QString KEY_CONFIRM_DELIVERY = "ConfirmDelivery";
const QString KEY_CONFIRM_READ = "ConfirmReading";
const QString KEY_ENCRYPT = "Encrypt";
const QString KEY_INBOX_FOLDER = "InboxFolder";
const QString KEY_DRAFTS_FOLDER = "InboxFolder";
const QString KEY_QUEUE_FOLDER = "QueueFolder";
const QString KEY_SENT_FOLDER = "SentFolder";
const QString KEY_TRASH_FOLDER = "TrashFolder";
const QString KEY_CC_OTHER = "CopyOther";
const QString KEY_CC_OTHER_ADDRESS = "CopyOtherAddress";
const QString KEY_LOCAL_MAILBOX_PATH = "MailboxPath";
const QString KEY_ADDRESSBOOK_FILENAME = "AddressbookFilename";

// UI
const QString KEY_MAIN_WINDOW_TOOLBAR_POSITION = "MainWindowToolbarPosition";
const QString KEY_COMPOSE_WINDOW_TOOLBAR_POSITION = "ComposeWindowToolbarPos";
const QString KEY_TIP_OF_THE_DAY_AT_STARTUP = "TipOfTheDayAtStartup";
const QString KEY_MESSAGE_LIST_SIZE_COLUMN = "MessageListSizeOfColumn";
const QString KEY_MESSAGE_LIST_POS_COLUMN = "MessageListPositionOfColumn";
const QString KEY_FIXED_FONT = "FixedFont";
const QString KEY_UNDERLINE_LINKS = "UnderlineLinks";
const QString KEY_QUOTE_COLOUR_ONE = "QuoteColourOne";
const QString KEY_QUOTE_COLOUR_TWO = "QuoteColourTwo";
const QString KEY_LINK_COLOUR = "LinkColour";
const QString KEY_VISITED_LINK_COLOUR = "VisitedLinkColour";
const QString KEY_ICON_SET = "IconSet";
const QString KEY_USE_EXTERNAL_EDITOR = "UseExternalEditor";
const QString KEY_EXTERNAL_EDITOR = "ExternalEditor";
const QString KEY_STAR_PASSWORD = "StarPassword";
const QString KEY_THREAD_MESSAGES = "ThreadMessages";
const QString KEY_SHOW_HEADERS = "ShowHeaders";
const QString KEY_MAIN_WINDOW_X_SIZE = "MainWindowXSize";
const QString KEY_MAIN_WINDOW_Y_SIZE = "MainWindowYSize";
const QString KEY_MAIN_WIDGET_V_SEP = "MainWidgetVerticalSeparatorPosition";
const QString KEY_MAIN_WIDGET_H_SEP = "MainWidgetHorizontalSeparatorPosition";
const QString KEY_MESSAGE_SORT_COLUMN = "MessageListSortColumn";
const QString KEY_MESSAGE_SORT_ASCENDING = "MessageListSortAscending";
const QString KEY_MARK_AS_READ = "MarkMessagesAsRead";
const QString KEY_MARK_AS_READ_TIME = "MarkMessagesAsReadAfterTime";
const QString KEY_FOLDER_ITEMS_OPEN = "FolderListItemsOpen";
const QString KEY_EXTRA_HEADERS = "ExtraHeaders";

// Defaults
const bool          DEFAULT_UNDERLINE_LINKS = true;
const QString       DEFAULT_SIG_FILE = "~/.signature";
const QString       DEFAULT_PHRASE_REPLY_TO_SEND = "On %d, you wrote:";
const QString       DEFAULT_PHRASE_REPLY_TO_ALL = "On %d, %n wrote:";
const QString       DEFAULT_PHRASE_FORWARD = "Forwarded message from %n";
const bool          DEFAULT_AUTO_QUOTE_WHEN_REPLY = true;
const bool          DEFAULT_APPEND_SIG = true;
const bool          DEFAULT_APPEND_DIG_SIG = false;
const unsigned int  DEFAULT_WRAP_COLUMN = 80;
const bool          DEFAULT_CC_ME = false;
const bool          DEFAULT_CC_OTHER = false;
const QString       DEFAULT_COPY_FOLDER = "outbox";
const bool          DEFAULT_SEND_NOW = true;
const unsigned int  DEFAULT_OUT_SERVER_PORT = 25;
const bool          DEFAULT_CHECK_FOR_NEW_MAIL = true;
const unsigned int  DEFAULT_CHECK_NEW_INTERVAL = 10;
const QString       DEFAULT_MAIL_SAVE_FOLDER = "~/.empath";

}

// vim:ts=4:sw=4:tw=78
