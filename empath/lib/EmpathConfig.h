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

#include <qstring.h>

class EmpathConfig
{
	public:
		static const QString		GROUP_GENERAL;
		static const QString		GROUP_MAILBOX;
		static const QString		GROUP_FILTER;
		static const QString		GROUP_ACTION;
		static const QString		GROUP_EXPR;
		static const QString		GROUP_DISPLAY;
		static const QString		GROUP_COMPOSE;
		static const QString		GROUP_IDENTITY;
		static const QString		GROUP_SENDING;
		static const QString		GROUP_ACCOUNT;

		// DISPLAY
		static const QString		KEY_MAIN_WINDOW_TOOLBAR_POSITION;
		static const QString		KEY_COMPOSE_WINDOW_TOOLBAR_POSITION;
		static const QString		KEY_NUM_MAILBOXES;
		static const QString		KEY_TIP_OF_THE_DAY_AT_STARTUP;
		static const QString		KEY_MESSAGE_LIST_SIZE_COLUMN;
		static const QString		KEY_MESSAGE_LIST_POS_COLUMN;
		static const QString		KEY_MAILBOX_TYPE;
		static const QString		KEY_MAILBOX_NAME;
		static const QString		KEY_MAILBOX_LIST;
		static const QString		KEY_FOLDER_LIST;
		static const QString		KEY_FIXED_FONT;
		static const QString		KEY_UNDERLINE_LINKS;
		static const QString		KEY_QUOTE_COLOUR_ONE;
		static const QString		KEY_QUOTE_COLOUR_TWO;
		static const QString		KEY_LINK_COLOUR;
		static const QString		KEY_VISITED_LINK_COLOUR;
		static const QString		KEY_THREAD_MESSAGES;
		static const QString		KEY_MESSAGE_SORT_COLUMN;
		static const QString		KEY_MESSAGE_SORT_ASCENDING;
		static const QString		KEY_SHOW_HEADERS;
		static const QString		KEY_ICON_SET;
		static const QString		KEY_MARK_AS_READ;
		static const QString		KEY_MARK_AS_READ_TIME;
		static const QString		KEY_MAIN_WINDOW_X_SIZE;
		static const QString		KEY_MAIN_WINDOW_Y_SIZE;
		static const QString		KEY_MAIN_WIDGET_V_SEP;
		static const QString		KEY_MAIN_WIDGET_H_SEP;
		static const QString		KEY_FOLDER_ITEMS_OPEN;
		
		// IDENTITY
		static const QString		KEY_NAME;
		static const QString		KEY_EMAIL;
		static const QString		KEY_REPLY_TO;
		static const QString		KEY_ORGANISATION;
		static const QString		KEY_SIG_PATH;
		
		// COMPOSING
		static const QString		KEY_PHRASE_REPLY_SENDER;
		static const QString		KEY_PHRASE_REPLY_ALL;
		static const QString		KEY_PHRASE_FORWARD;
		static const QString		KEY_AUTO_QUOTE;
		static const QString		KEY_ADD_SIG;
		static const QString		KEY_ADD_DIG_SIG;
		static const QString		KEY_CONFIRM_DELIVERY;
		static const QString		KEY_CONFIRM_READ;
		static const QString		KEY_ENCRYPT;
		static const QString		KEY_WRAP_LINES;
		static const QString		KEY_WRAP_COLUMN;
		static const QString		KEY_USE_EXTERNAL_EDITOR;
		static const QString		KEY_EXTERNAL_EDITOR;
		
		// SENDING
		static const QString		KEY_CC_OTHER;
		static const QString		KEY_CC_OTHER_ADDRESS;
		static const QString		KEY_SEND_POLICY;
		static const QString		KEY_QUEUE_FOLDER;
		static const QString		KEY_SENT_FOLDER;
		static const QString		KEY_TRASH_FOLDER;
		static const QString		KEY_DRAFTS_FOLDER;
		
		// ACCOUNTS
		static const QString		KEY_ACCOUNT_TYPE;
		static const QString		KEY_SERVER_ADDRESS;
		static const QString		KEY_SERVER_PORT;
		static const QString		KEY_SERVER_UNAME;
		static const QString		KEY_SERVER_PASSWORD;
		static const QString		KEY_STAR_PASSWORD;
		static const QString		KEY_CHECK_MAIL;
		static const QString		KEY_CHECK_MAIL_INTERVAL;
		static const QString		KEY_INTERNAL_MAIL_FILE;
		static const QString		KEY_OUTGOING_SERVER_TYPE;
		static const QString		KEY_SENDMAIL_LOCATION;
		static const QString		KEY_QMAIL_LOCATION;
		static const QString		KEY_SMTP_SERVER_LOCATION;
		static const QString		KEY_SMTP_SERVER_PORT;
		static const QString		KEY_LOCAL_MAILBOX_PATH;
		
		// POP3
		static const QString		KEY_POP3_SERVER_ADDRESS;
		static const QString		KEY_POP3_SERVER_PORT;
		static const QString		KEY_POP3_USERNAME;
		static const QString		KEY_POP3_PASSWORD;
		static const QString		KEY_POP3_APOP;
		static const QString		KEY_POP3_SAVE_POLICY;
		static const QString		KEY_POP3_LOGGING_POLICY;
		static const QString		KEY_POP3_LOG_FILE_PATH;
		static const QString		KEY_POP3_LOG_FILE_DISPOSAL_POLICY;
		static const QString		KEY_POP3_MAX_LOG_FILE_SIZE;
		static const QString		KEY_POP3_MESSAGE_SIZE_THRESHOLD;
		static const QString		KEY_POP3_LARGE_MESSAGE_POLICY;
		static const QString		KEY_POP3_CHECK_FOR_NEW_MAIL;
		static const QString		KEY_POP3_MAIL_CHECK_INTERVAL;
		static const QString		KEY_POP3_DELETE_FROM_SERVER;
		static const QString		KEY_POP3_AUTO_GET_NEW_MAIL;
		static const QString		KEY_POP3_SAVE_ALL_ADDRESSES;
		static const QString		KEY_POP3_NOTIFY;
		static const QString		KEY_POP3_RETRIEVE_IF_HAVE;
		
		// FILTERS
		static const QString		KEY_FILTER_LIST;
		static const QString		KEY_NUM_MATCH_EXPRS_FOR_FILTER;
		static const QString		KEY_MATCH_EXPR_TYPE;
		static const QString		KEY_MATCH_SIZE;
		static const QString		KEY_MATCH_EXPR;
		static const QString		KEY_MATCH_HEADER;
		static const QString		KEY_FILTER_EVENT_HANDLER_TYPE;
		static const QString		KEY_FILTER_EVENT_HANDLER_FOLDER;
		static const QString		KEY_FILTER_EVENT_HANDLER_ADDRESS;
		static const QString		KEY_FILTER_FOLDER;
		static const QString		KEY_FILTER_PRIORITY;

		// DEFAULTS
		static const bool			DEFAULT_UNDERLINE_LINKS;
		static const QString		DEFAULT_SIG_FILE;
		static const QString		DEFAULT_PHRASE_REPLY_TO_SEND;
		static const QString		DEFAULT_PHRASE_REPLY_TO_ALL;
		static const QString		DEFAULT_PHRASE_FORWARD;
		static const bool			DEFAULT_AUTO_QUOTE_WHEN_REPLY;
		static const bool			DEFAULT_APPEND_SIG;
		static const bool			DEFAULT_APPEND_DIG_SIG;
		static const unsigned int 	DEFAULT_WRAP_COLUMN;
		static const bool			DEFAULT_CC_ME;
		static const bool			DEFAULT_CC_OTHER;
		static const QString		DEFAULT_COPY_FOLDER;
		static const bool			DEFAULT_SEND_NOW;
		static const unsigned int 	DEFAULT_OUT_SERVER_PORT;
		static const bool			DEFAULT_CHECK_FOR_NEW_MAIL;
		static const unsigned int 	DEFAULT_CHECK_NEW_INTERVAL;
		static const QString		DEFAULT_MAIL_SAVE_FOLDER;
};

#endif
