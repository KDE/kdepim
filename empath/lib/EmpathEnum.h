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

#ifndef EMPATHENUM_H
#define EMPATHENUM_H

enum WriteToDisk		{ Write, NoWrite };
	
enum OutgoingServerType	{ Sendmail, Qmail, SMTP, QMTP };
enum DialogRetval		{ OK, Cancel, Help, Defaults };
enum FontStyle			{ Fixed, Variable };
enum AccountType		{ Local, Maildir, POP3, IMAP4 };
enum SendPolicy			{ SendNow, SendLater };
enum SavePolicy			{ Forever, ThisSession, Never };
enum LargeMessagePolicy	{ RetrieveMessage, LeaveOnServer, RetrieveHeader };
enum ActionType			{ MoveFolder, CopyFolder, Delete, Ignore, Forward };
enum MatchExprType		{ Size, BodyExpr, HeaderExpr, HasAttachments, AnyMessage };
enum LockType			{ LockRead, LockWrite };
enum ComposeType		{ ComposeReply, ComposeReplyAll, ComposeForward, ComposeNormal };

enum SettingsSection	{
	DisplaySettings		= 1 << 0,
	IdentitySettings	= 1 << 1,
	ComposeSettings		= 1 << 2,
	SendingSettings		= 1 << 3,
	AccountsSettings	= 1 << 4,
	FilterSettings		= 1 << 5,
	AllSettings			= 0xffffffff
};

#endif

