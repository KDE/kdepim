/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

// Groups

// Stuff that is not set by the user
#define GROUP_GENERAL			"General"

// Not a real group - we append a number to make a canonical group name
// for each mailbox
#define GROUP_MAILBOX			"Mailbox_"

// Similar
#define GROUP_FILTER			"Filter_"
#define GROUP_ACTION			"Action_"
#define GROUP_EXPR				"Expr_"

// User defined stuff - from the Settings dialog
#define GROUP_DISPLAY			"Display"
#define GROUP_COMPOSE			"Compose"
#define GROUP_IDENTITY			"Identity"
#define GROUP_SENDING			"Sending"
#define GROUP_ACCOUNT			"Account"

// Keys

// General
#define KEY_MAIN_WINDOW_TOOLBAR_POS		"MainWindowToolbarPosition"
#define KEY_COMPOSE_WINDOW_TOOLBAR_POS	"ComposeWindowToolbarPosition"
#define KEY_NUM_MAILBOXES				"NumberOfMailboxes"
#define KEY_TIP_OF_THE_DAY_AT_STARTUP	"TipOfTheDayAtStartup"

// Message list
#define KEY_MESSAGE_LIST_SIZE_COL		"MessageListSizeOfColumn"

#define KEY_MESSAGE_LIST_POS_COL		"MessageListPositionOfColumn"


// Mailbox
#define	KEY_MAILBOX_TYPE		"MailboxType"
#define	KEY_MAILBOX_NAME		"MailboxName"
#define KEY_MAILBOX_LIST		"MailboxList"
#define KEY_FOLDER_LIST			"FolderList"

// Display

#define KEY_VARIABLE_FONT		"VariableWidthFont"
#define KEY_FIXED_FONT			"FixedWidthFont"
#define KEY_QUOTED_FONT			"QuotedFont"

#define KEY_UNDERLINE_LINKS		"UnderlineLinks"

#define KEY_FONT_STYLE			"FontStyle"

#define KEY_BACKGROUND_COLOUR	"BackgroundColour"
#define KEY_TEXT_COLOUR			"TextColour"
#define KEY_LINK_COLOUR			"LinkColour"
#define KEY_VISITED_LINK_COLOUR	"VisitedLinkColour"

#define KEY_USE_DEFAULT_FONTS	"UseDefaultFonts"
#define KEY_USE_DEFAULT_COLOURS	"UseDefaultColours"

#define KEY_ICON_SET			"IconSet"

// Identity

#define KEY_NAME				"Name"
#define KEY_EMAIL				"Email"
#define KEY_REPLY_TO			"ReplyTo"
#define KEY_ORGANISATION		"Organisation"
#define KEY_SIG_PATH			"Signature"

#define KEY_PHRASE_REPLY_SENDER	"PhraseReplySender"
#define KEY_PHRASE_REPLY_ALL	"PhraseReplyAll"
#define KEY_PHRASE_FORWARD		"PhraseForward"

// Sending

#define KEY_AUTO_QUOTE			"AutoQuoteOnReply"
#define KEY_ADD_SIG				"AddSignature"
#define KEY_ADD_DIG_SIG			"AddDigitalSignature"
#define KEY_WRAP_LINES			"WrapLongLines"
#define KEY_WRAP_COLUMN			"WrapColumn"

#define KEY_CC_ME				"CCToMe"
#define KEY_CC_OTHER			"CCToOther"
#define KEY_CC_OTHER_ADDRESS	"CCOtherAddress"
#define KEY_COPY_FOLDER			"CopyFolder"
#define KEY_COPY_FOLDER_NAME	"CopyFolderName"

#define KEY_SEND_POLICY			"SendWhen"

#define KEY_USE_EXTERNAL_EDITOR	"UseExternalEditor"
#define KEY_EXTERNAL_EDITOR		"ExternalEditor"

#define KEY_ACCOUNT_TYPE		"AccountType"
#define KEY_SERVER_ADDRESS		"ServerAddress"
#define KEY_SERVER_PORT			"ServerPort"
#define KEY_SERVER_UNAME		"ServerUsername"
#define KEY_SERVER_PASSWORD		"ServerPassword"
#define KEY_STAR_PASSWORD		"StarPassword"
#define KEY_CHECK_MAIL			"CheckMailAtIntervals"
#define KEY_CHECK_MAIL_INTERVAL	"CheckMailInterval"

#define KEY_INTERNAL_MAIL_FILE	"EmpathMaildir"

// Outgoing server

#define KEY_OUTGOING_SERVER_TYPE	"OutgoingServerType"
#define KEY_SENDMAIL_LOCATION		"SendmailLocation"
#define KEY_QMAIL_LOCATION			"QmailLocation"
#define KEY_SMTP_SERVER_LOCATION	"SMTPServerLocation"
#define KEY_SMTP_SERVER_PORT		"SMTPServerPort"
#define KEY_QMTP_SERVER_LOCATION	"QMTPServerLocation"
#define KEY_QMTP_SERVER_PORT		"QMTPServerPort"

// Local

#define KEY_LOCAL_MAILBOX_PATH				"MailboxPath"
#define KEY_LOCAL_CHECK_FOR_NEW_MAIL		"CheckForNewMail"
#define KEY_LOCAL_MAIL_CHECK_INTERVAL		"MailCheckInterval"
#define KEY_LOCAL_AUTO_GET_NEW_MAIL			"AutoGetNewMail"
#define KEY_LOCAL_SAVE_ALL_ADDRESSES		"SaveAllAddresses"
#define KEY_LOCAL_NOTIFY					"Notify"

// POP3

#define KEY_POP3_SERVER_ADDRESS				"ServerAddress"
#define KEY_POP3_SERVER_PORT				"ServerPort"
#define KEY_POP3_USERNAME					"Username"
#define KEY_POP3_PASSWORD					"Password"
#define KEY_POP3_APOP						"UseAPOP"
#define KEY_POP3_SAVE_POLICY				"SavePolicy"
#define KEY_POP3_LOGGING_POLICY				"LoggingPolicy"
#define KEY_POP3_LOG_FILE_PATH				"LogFilePath"
#define KEY_POP3_LOG_FILE_DISPOSAL_POLICY	"LogFileDisposalPolicy"
#define KEY_POP3_MAX_LOG_FILE_SIZE			"MaxLogFileSize"
#define KEY_POP3_MESSAGE_SIZE_THRESHOLD		"MessageSizeThreshold"
#define KEY_POP3_LARGE_MESSAGE_POLICY		"LargeMessagePolicy"
#define KEY_POP3_CHECK_FOR_NEW_MAIL			"CheckForNewMail"
#define KEY_POP3_MAIL_CHECK_INTERVAL		"MailCheckInterval"
#define KEY_POP3_DELETE_FROM_SERVER			"DeleteFromServer"
#define KEY_POP3_AUTO_GET_NEW_MAIL			"AutoGetNewMail"
#define KEY_POP3_SAVE_ALL_ADDRESSES			"SaveAllAddresses"
#define KEY_POP3_NOTIFY						"Notify"
#define KEY_POP3_RETRIEVE_IF_HAVE			"RetrieveIfHave"

#define KEY_NUMBER_OF_FILTERS				"NumberOfFilters"
#define KEY_NUM_MATCH_EXPRS_FOR_FILTER		"NumberOfMatchExpressions"
#define KEY_MATCH_EXPR_TYPE					"MatchExpressionType"
#define KEY_MATCH_SIZE						"MatchSize"
#define KEY_MATCH_EXPR						"MatchExpr"
#define KEY_MATCH_HEADER					"MatchHeaderExpr"
#define KEY_FILTER_EVENT_HANDLER_TYPE		"FilterEventHandlerType"
#define KEY_FILTER_EVENT_HANDLER_FOLDER		"FilterEventHandlerFolder"
#define KEY_FILTER_EVENT_HANDLER_ADDRESS	"FilterEventHandlerAddress"
#define KEY_FILTER_FOLDER					"FilterSourceFolder"

// Defaults

#define DEFAULT_UNDERLINE_LINKS		true
#define DEFAULT_FIXED_OR_VARIABLE	"fixed"

#define DEFAULT_SIG_FILE	"~/.signature"

#define	DEFAULT_PHRASE_REPLY_TO_SENDER	"On %d, you wrote:"
#define	DEFAULT_PHRASE_REPLY_TO_ALL		"On %d, %n wrote:"
#define	DEFAULT_PHRASE_FORWARD			"Forwarded message from %n"

#define DEFAULT_AUTO_QUOTE_WHEN_REPLY	true
#define DEFAULT_APPEND_SIG				true
#define DEFAULT_APPEND_DIG_SIG			false
#define DEFAULT_WRAP_COLUMN				80

#define DEFAULT_CC_ME		false
#define DEFAULT_CC_OTHER	false
#define DEFAULT_COPY_FOLDER	"outbox"

#define DEFAULT_SEND_NOW	true

#define	DEFAULT_OUT_SERVER_PORT		25

#define DEFAULT_CHECK_FOR_NEW_MAIL	true
#define DEFAULT_CHECK_NEW_INTERVAL	10

#define DEFAULT_MAIL_SAVE_FOLDER	"~/.empath"

#endif
