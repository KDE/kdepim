/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rikkus@postmaster.co.uk
	
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

#ifndef RMM_ENUM_H
#define RMM_ENUM_H

#include <qstring.h>

enum MessageStatus {
	Read		= 1 << 0,
	Marked		= 1 << 1,
	Trashed		= 1 << 2,
	Replied		= 1 << 3
};

static char * dayNames[] = {
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun"
};

static char * monthNames[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

static char * headerNames[] = {
	"Approved",
	"Bcc",
	"Cc",
	"Comments",
	"Content-Description",
	"Content-Disposition",
	"Content-ID",
	"Content-MD5",
	"Content-Type",
	"Control",
	"Content-Transfer-Encoding",
	"Date",
	"Distribution",
	"Encrypted",
	"Expires",
	"Followup-To",
	"From",
	"In-Reply-To",
	"Keywords",
	"Lines",
	"Message-ID",
	"Mime-Version",
	"Newsgroups",
	"Organization",
	"Path",
	"Received",
	"References",
	"Reply-To",
	"Resent-Bcc",
	"Resent-Cc",
	"Resent-Date",
	"Resent-From",
	"Resent-MessageID",
	"Resent-ReplyTo",
	"Resent-Sender",
	"Resent-To",
	"Return-Path",
	"Sender",
	"Subject",
	"Summary",
	"To",
	"Xref",
	"" // HeaderUnknown
};

enum HeaderType {
	HeaderApproved,
	HeaderBcc,
	HeaderCc,
	HeaderComments,
	HeaderContentDescription,
	HeaderContentDisposition,
	HeaderContentID,
	HeaderContentMD5,
	HeaderContentType,
	HeaderControl,
	HeaderContentTransferEncoding,
	HeaderDate,
	HeaderDistribution,
	HeaderEncrypted,
	HeaderExpires,
	HeaderFollowupTo,
	HeaderFrom,
	HeaderInReplyTo,
	HeaderKeywords,
	HeaderLines,
	HeaderMessageID,
	HeaderMimeVersion,
	HeaderNewsgroups,
	HeaderOrganization,
	HeaderPath,
	HeaderReceived,
	HeaderReferences,
	HeaderReplyTo,
	HeaderResentBcc,
	HeaderResentCc,
	HeaderResentDate,
	HeaderResentFrom,
	HeaderResentMessageID,
	HeaderResentReplyTo,
	HeaderResentSender,
	HeaderResentTo,
	HeaderReturnPath,
	HeaderSender,
	HeaderSubject,
	HeaderSummary,
	HeaderTo,
	HeaderXref,
	HeaderUnknown
};

enum HeaderDataType {
	Address,
	AddressList,
	DateTime,
	DispositionType,
	Mailbox,
	MailboxList,
	Mechanism,
	MessageID,
	Text
};

	static HeaderDataType
headerTypesTable[] = {
	Text,			// HeaderApproved
	AddressList,	// HeaderBcc
	MailboxList,	// HeaderCc
	Text,			// HeaderComments
	Text,			// HeaderContentDescription
	DispositionType,// HeaderContentDisposition
	MessageID,		// HeaderContentID
	Text,			// HeaderContentMD5
	Text,			// HeaderContentType
	Text,			// HeaderControl
	Text,			// HeaderContentTransferEncoding
	DateTime,		// HeaderDate
	Text,			// HeaderDistribution
	Text,			// HeaderEncrypted
	DateTime,		// HeaderExpires
	Text,			// HeaderFollowupTo
	MailboxList,	// HeaderFrom
	Text,			// HeaderInReplyTo
	Text,			// HeaderKeywords
	Text,			// HeaderLines
	MessageID,		// HeaderMessageID
	Text,			// HeaderMimeVersion
	Text,			// HeaderNewsgroups
	Text,			// HeaderOrganization
	Text,			// HeaderPath
	Text,			// HeaderReceived
	Text,			// HeaderReferences
	AddressList,	// HeaderReplyTo
	AddressList,	// HeaderResentBcc
	AddressList,	// HeaderResentCc
	DateTime,		// HeaderResentDate
	MailboxList,	// HeaderResentFrom
	MessageID,		// HeaderResentMessageID
	AddressList,	// HeaderResentReplyTo
	Mailbox,		// HeaderResentSender
	AddressList,	// HeaderResentTo
	Text,			// HeaderReturnPath
	Mailbox,		// HeaderSender
	Text,			// HeaderSubject
	Text,			// HeaderSummary
	AddressList,	// HeaderTo
	Text,			// HeaderXref
	Text			// HeaderUnknown
};

enum CteType {
	CteType7bit,
	CteType8bit,
	CteTypeBinary,
	CteTypeQuotedPrintable,
	CteTypeBase64,
	CteTypeXtension
};

enum MimeType {
	MimeTypeNone,
	MimeTypeUnknown,
	MimeTypeText,
	MimeTypeMultiPart,
	MimeTypeMessage,
	MimeTypeApplication,
	MimeTypeImage,
	MimeTypeAudio,
	MimeTypeVideo,
	MimeTypeModel
};

enum MimeSubType {
	MimeSubTypeNone,
	MimeSubTypeUnknown,
	MimeSubTypePlain,
	MimeSubTypeRichtext,
	MimeSubTypeEnriched,
	MimeSubTypeHTML,
	MimeSubTypeMixed,
	MimeSubTypeAlternative,
	MimeSubTypeDigest,
	MimeSubTypeParallel,
	MimeSubTypeRFC822,
	MimeSubTypePartial,
	MimeSubTypeExternalBody,
	MimeSubTypePostScript,
	MimeSubTypeOctetStream,
	MimeSubTypeJpeg,
	MimeSubTypeGIF,
	MimeSubTypeBasic,
	MimeSubTypeMPEG,
};

enum DispType {
	DispositionTypeInline,
	DispositionTypeAttachment
};

enum DayOfWeek {
	DayNone,
	DayMon, DayTue, DayWed, DayThu, DayFri, DaySat, DaySun
};

DayOfWeek strToDayOfWeek(const QCString & s);

enum Month {
	MonthJan, MonthFeb, MonthMar, MonthApr, MonthMay, MonthJun,
	MonthJul, MonthAug, MonthSep, MonthOct, MonthNov, MonthDec
};

Month strToMonth(const QCString & s);

MimeType	mimeTypeStr2Enum(const QCString & s);
QCString		mimeTypeEnum2Str(MimeType m);
MimeSubType	mimeSubTypeStr2Enum(const QCString & s);
QCString		mimeSubTypeEnum2Str(MimeSubType t);
CteType		RCteStr2Enum(const QCString & s);
QCString		cteTypeEnumToStr(CteType t);

#endif
