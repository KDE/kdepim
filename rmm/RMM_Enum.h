/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef RMM_ENUM_H
#define RMM_ENUM_H

#include <qstring.h>
#include <qcstring.h>

namespace RMM {

enum MessageStatus {
    Read    = 1 << 0,
    Marked  = 1 << 1,
    Trashed = 1 << 2,
    Replied = 1 << 3
};

extern const QCString monthNames[];
extern const QCString headerNames[];

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
    ContentType,
    Cte,
    DateTime,
    ContentDisposition,
    Mechanism,
    MessageID,
    Text
};

extern const HeaderDataType headerTypesTable[];

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
    MimeSubTypeXcpio,
    MimeSubTypeXdvi,
    MimeSubTypeXperl,
    MimeSubTypeXtar,
    MimeSubTypeXdeb,
    MimeSubTypeXrar,
    MimeSubTypeXlatex,
    MimeSubTypeXsh,
    MimeSubTypeXshar,
    MimeSubTypeXtgz,
    MimeSubTypeXtcl,
    MimeSubTypeXtex,
    MimeSubTypeXtroff,
    MimeSubTypeXzip,
    MimeSubTypeXras,
    MimeSubTypeXpnm,
    MimeSubTypeXpbm,
    MimeSubTypeXpgm,
    MimeSubTypeXppm,
    MimeSubTypeXrgb,
    MimeSubTypeXxbm,
    MimeSubTypeXxpm,
    MimeSubTypeXavi,
    MimeSubTypeXsgi,
    MimeSubTypeXaiff,
    MimeSubTypeXwav,
    MimeSubTypePNG,
    MimeSubTypeTIFF,
    MimeSubTypeGL,
    MimeSubTypeFLI,
    MimeSubTypeQuickTime,
    MimeSubTypeXvrml,
    MimeSubTypeMIDI,
    MimeSubTypeULAW
};

enum DispType {
    DispositionTypeInline,
    DispositionTypeAttachment
};

enum DayOfWeek {
    DayNone,
    DayMon, DayTue, DayWed, DayThu, DayFri, DaySat, DaySun
};

// DayOfWeek strToDayOfWeek(const QCString & s);

enum Month {
    MonthJan, MonthFeb, MonthMar, MonthApr, MonthMay, MonthJun,
    MonthJul, MonthAug, MonthSep, MonthOct, MonthNov, MonthDec
};

Month       strToMonth(const QCString & s);

MimeType    mimeTypeStr2Enum(const QCString & s);
QCString    mimeTypeEnum2Str(MimeType m);
MimeSubType mimeSubTypeStr2Enum(const QCString & s);
QCString    mimeSubTypeEnum2Str(MimeSubType t);
CteType     RCteStr2Enum(const QCString & s);
QCString    cteTypeEnumToStr(CteType t);

QString     mimeTypeToIconName(MimeType t, MimeSubType st);

HeaderType        headerNameToEnum(const QCString & s);
HeaderDataType    headerNameToType(const QCString & s);
QCString            headerTypeToName(HeaderType);

}

#endif
// vim:ts=4:sw=4:tw=78
