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

#include <qstring.h>

#include <RMM_Enum.h>

using namespace RMM;

    const QCString
RMM::monthNames [] = {
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

    const QCString
RMM::headerNames [] = {
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


    const HeaderDataType
RMM::headerTypesTable [] = {
    Text,                // HeaderApproved
    AddressList,        // HeaderBcc
    AddressList,        // HeaderCc
    Text,                // HeaderComments
    Text,                // HeaderContentDescription
    DispositionType,    // HeaderContentDisposition
    MessageID,            // HeaderContentID
    Text,                // HeaderContentMD5
    ContentType,        // HeaderContentType
    Text,                // HeaderControl
    Cte,                // HeaderContentTransferEncoding
    DateTime,            // HeaderDate
    Text,                // HeaderDistribution
    Text,                // HeaderEncrypted
    DateTime,            // HeaderExpires
    Text,                // HeaderFollowupTo
    AddressList,        // HeaderFrom
    Text,                // HeaderInReplyTo
    Text,                // HeaderKeywords
    Text,                // HeaderLines
    MessageID,            // HeaderMessageID
    Text,                // HeaderMimeVersion
    Text,                // HeaderNewsgroups
    Text,                // HeaderOrganization
    Text,                // HeaderPath
    Text,                // HeaderReceived
    Text,                // HeaderReferences
    AddressList,        // HeaderReplyTo
    AddressList,        // HeaderResentBcc
    AddressList,        // HeaderResentCc
    DateTime,            // HeaderResentDate
    AddressList,        // HeaderResentFrom
    MessageID,            // HeaderResentMessageID
    AddressList,        // HeaderResentReplyTo
    Address,            // HeaderResentSender
    AddressList,        // HeaderResentTo
    Text,                // HeaderReturnPath
    Address,            // HeaderSender
    Text,                // HeaderSubject
    Text,                // HeaderSummary
    AddressList,        // HeaderTo
    Text,                // HeaderXref
    Text                // HeaderUnknown
};

    Month
RMM::strToMonth(const QCString & s)
{
    if (s.isEmpty()) return MonthJan;

    for (int i = 1; i <= 12; i++)
        if (!stricmp(s, monthNames[i-1])) return (Month)i;

    return MonthJan;

}

    MimeType
RMM::mimeTypeStr2Enum(const QCString & s)
{
    if (s.isEmpty()) return MimeTypeNone;

    MimeType t = MimeTypeUnknown;

    switch (s[0]) {

        case 0:

            t = MimeTypeNone;

        case 'a':
        case 'A':

            if (stricmp(s, "application") == 0)
                t = MimeTypeApplication;

            else if (stricmp(s, "audio") == 0)
                t = MimeTypeAudio;

            break;

        case 'i':
        case 'I':

            if (stricmp(s, "image") == 0)
                t = MimeTypeImage;

            break;

        case 'm':
        case 'M':

            if (stricmp(s, "message") == 0)
                t = MimeTypeMessage;

            else if (stricmp(s, "multipart") == 0)
                t = MimeTypeMultiPart;

            break;

        case 't':
        case 'T':

            if (stricmp(s, "text") == 0)
                t = MimeTypeText;

            break;

        case 'v':
        case 'V':

            if (stricmp(s, "video") == 0)
                t = MimeTypeVideo;

            break;

        default:

            t = MimeTypeUnknown;
    }

    return t;
}

    QCString
RMM::mimeTypeEnum2Str(MimeType m)
{
    QCString s;

    switch (m) {

        case MimeTypeNone:
            s = "Unknown";
            break;

        case MimeTypeUnknown:
            s = "Unknown";
            break;

        case MimeTypeText:
            s = "Text";
            break;

        case MimeTypeMultiPart:
            s = "Multipart";
            break;

        case MimeTypeMessage:
            s = "Message";
            break;

        case MimeTypeImage:
            s = "Image";
            break;

        case MimeTypeApplication:
            s = "Application";
            break;

        case MimeTypeAudio:
            s = "Audio";
            break;

        case MimeTypeVideo:
            s = "Video";
            break;

        case MimeTypeModel:
            s = "Model";
            break;

        default:
            s = "Unknown";
            break;
    }

    return s;
}

    MimeSubType
RMM::mimeSubTypeStr2Enum(const QCString & s)
{
    if (s.isEmpty()) return MimeSubTypeNone;

    MimeSubType st = MimeSubTypeNone;

    switch (s[0]) {

        case 'a':
        case 'A':

            if (stricmp(s, "alternative") == 0)
                st = MimeSubTypeAlternative;

            break;

        case 'b':
        case 'B':

            if (stricmp(s, "basic") == 0)
                st = MimeSubTypeBasic;

            break;

        case 'd':
        case 'D':

            if (stricmp(s, "digest") == 0)
                st = MimeSubTypeDigest;

            break;

        case 'e':
        case 'E':

            if (stricmp(s, "enriched") == 0)
                st = MimeSubTypeEnriched;

            else if (stricmp(s, "external-body") == 0)
                st = MimeSubTypeExternalBody;

            break;

        case 'g':
        case 'G':

            if (stricmp(s, "gif") == 0)
                st = MimeSubTypeGIF;

            break;

        case 'h':
        case 'H':

            if (stricmp(s, "html") == 0)
                st = MimeSubTypeHTML;

            break;

        case 'j':
        case 'J':

            if (stricmp(s, "jpeg") == 0)
                st = MimeSubTypeJpeg;

            break;

        case 'm':
        case 'M':

            if (stricmp(s, "mixed") == 0)
                st = MimeSubTypeMixed;

            else if (stricmp(s, "mpeg") == 0)
                st = MimeSubTypeMPEG;

            break;

        case 'o':
        case 'O':

            if (stricmp(s, "octet-stream") == 0)
                st = MimeSubTypeOctetStream;

            break;

        case 'p':
        case 'P':

            if (stricmp(s, "plain") == 0)
                st = MimeSubTypePlain;

            else if (stricmp(s, "png") == 0)
                st = MimeSubTypePNG;
            
            else if (stricmp(s, "postscript") == 0)
                st = MimeSubTypePostScript;

            else if (stricmp(s, "parallel") == 0)
                st = MimeSubTypeParallel;

            else if (stricmp(s, "partial") == 0)
                st = MimeSubTypePartial;

            break;

        case 'r':
        case 'R':

            if (stricmp(s, "rfc822") == 0)
                st = MimeSubTypeRFC822;

            else if (stricmp(s, "richtext") == 0)
                st = MimeSubTypeRichtext;

            break;
            
        case 'x':
        case 'X':

            switch (s[2]) {
                
                case 'A':
                case 'a':
                    
                    if (stricmp(s, "x-msvideo") == 0)
                        st = MimeSubTypeXavi;
            
                    else if (stricmp(s, "x-aiff") == 0)
                        st = MimeSubTypeXaiff;

                    break;
                
                case 'C':
                case 'c':
                
                    if (stricmp(s, "x-cpio") == 0)
                        st = MimeSubTypeXcpio;
                    break;

                case 'D':
                case 'd':
                    
                    if (stricmp(s, "x-dvi") == 0)
                        st = MimeSubTypeXdvi;
                    else if (stricmp(s, "x-deb") == 0)
                        st = MimeSubTypeXdeb;
                    break;

                case 'L':
                case 'l':
                
                    if (stricmp(s, "x-latex") == 0)
                        st = MimeSubTypeXlatex;
                    break;

                case 'P':
                case 'p':
                
                    if (stricmp(s, "x-perl") == 0)
                        st = MimeSubTypeXperl;

                    else if (stricmp(s, "x-portable-anymap") == 0)
                        st = MimeSubTypeXpnm;
                    
                    else if (stricmp(s, "x-portable-bitmap") == 0)
                        st = MimeSubTypeXpbm;
                    
                    else if (stricmp(s, "x-portable-graymap") == 0)
                        st = MimeSubTypeXpgm;
                    
                    else if (stricmp(s, "x-portable-pixmap") == 0)
                        st = MimeSubTypeXppm;

                    break;
                
                case 'R':
                case 'r':

                    if (stricmp(s, "x-rar") == 0)
                        st = MimeSubTypeXrar;

                    else if (stricmp(s, "x-cmu-raster") == 0)
                        st = MimeSubTypeXras;
            
                    else if (stricmp(s, "x-rgb") == 0)
                        st = MimeSubTypeXrgb;

                    break;

                case 'S':
                case 's':
    
                    if (stricmp(s, "x-sh") == 0)
                        st = MimeSubTypeXsh;

                    else if (stricmp(s, "x-shar") == 0)
                        st = MimeSubTypeXshar;
                    
                    else if (stricmp(s, "x-sgi-movie") == 0)
                        st = MimeSubTypeXsgi;

                    break;
    
                case 'T':
                case 't':
                    
                    if (stricmp(s, "x-tar") == 0)
                        st = MimeSubTypeXtar;

                    else if (stricmp(s, "x-tgz") == 0)
                        st = MimeSubTypeXtgz;

                    else if (stricmp(s, "x-tcl") == 0)
                        st = MimeSubTypeXtcl;

                    else if (stricmp(s, "x-tex") == 0)
                        st = MimeSubTypeXtex;

                    else if (stricmp(s, "x-troff") == 0)
                        st = MimeSubTypeXtroff;

                    break;
                    
                case 'X':
                case 'x':

                    if (stricmp(s, "x-xbitmap") == 0)
                        st = MimeSubTypeXxbm;
                    
                    else if (stricmp(s, "x-xpixmap") == 0)
                        st = MimeSubTypeXxpm;
                
                    break;
            
                case 'W':
                case 'w':
                    
                    if (stricmp(s, "x-wav") == 0)
                        st = MimeSubTypeXwav;
                    break;
                
                case 'Z':
                case 'z':

                    if (stricmp(s, "x-zip") == 0)
                        st = MimeSubTypeXzip;
                    break;
    
                    
                default:
                    break;
            }

            break;

        default:

            st = MimeSubTypeUnknown;
            break;
    }

    return st;
}

    QCString
RMM::mimeSubTypeEnum2Str(MimeSubType t)
{
    QCString s;

    switch (t) {

        case MimeSubTypeNone:
            s = "Unknown";
            break;


        case MimeSubTypePlain:
            s = "Plain";
            break;

        case MimeSubTypeRichtext:
            s = "Richtext";
            break;

        case MimeSubTypeEnriched:
            s = "Enriched";
            break;

        case MimeSubTypeHTML:
            s = "HTML";
            break;

        case MimeSubTypeMixed:
            s = "Mixed";
            break;

        case MimeSubTypeAlternative:
            s = "Alternative";
            break;

        case MimeSubTypeDigest:
            s = "Digest";
            break;

        case MimeSubTypeParallel:
            s = "Parallel";
            break;

        case MimeSubTypeRFC822:
            s = "RFC822";
            break;

        case MimeSubTypePartial:
            s = "Partial";
            break;

        case MimeSubTypeExternalBody:
            s = "External-body";
            break;

        case MimeSubTypePostScript:
            s = "Postscript";
            break;

        case MimeSubTypeOctetStream:
            s = "Octet-stream";
            break;

        case MimeSubTypeJpeg:
            s = "JPEG";
            break;

        case MimeSubTypeGIF:
            s = "GIF";
            break;

        case MimeSubTypeBasic:
            s = "Basic";
            break;

        case MimeSubTypeMPEG:
            s = "MPEG";
            break;
            
        case MimeSubTypeXcpio:
            s = "X-cpio";
            break;

        case MimeSubTypeXdvi:
            s = "X-dvi";
            break;

        case MimeSubTypeXperl:
            s = "X-perl";
            break;

        case MimeSubTypeXtar:
            s = "X-tar";
            break;

        case MimeSubTypeXdeb:
            s = "X-deb";
            break;

        case MimeSubTypeXrar:
            s = "X-rar-compressed";
            break;

        case MimeSubTypeXlatex:
            s = "X-latex";
            break;

        case MimeSubTypeXsh:
            s = "X-sh";
            break;

        case MimeSubTypeXshar:
            s = "X-shar";
            break;

        case MimeSubTypeXtgz:
            s = "X-tar-gz";
            break;

        case MimeSubTypeXtcl:
            s = "X-tcl";
            break;

        case MimeSubTypeXtex:
            s = "X-tex";
            break;

        case MimeSubTypeXtroff:
            s = "X-troff";
            break;

        case MimeSubTypeXzip:
            s = "X-zip";
            break;
            
        case MimeSubTypeXras:
            s = "X-cmu-raster";
            break;

        case MimeSubTypeXpnm:
            s = "X-portable-anymap";
            break;

        case MimeSubTypeXpbm:
            s = "X-portable-bitmap";
            break;

        case MimeSubTypeXpgm:
            s = "X-portable-graymap";
            break;

        case MimeSubTypeXppm:
            s = "X-portable-pixmap";
            break;

        case MimeSubTypeXrgb:
            s = "X-rgb";
            break;

        case MimeSubTypeXxbm:
            s = "X-xbitmap";
            break;

        case MimeSubTypeXxpm:
            s = "X-xpixmap";
            break;

        case MimeSubTypeXavi:
            s = "X-msvideo";
            break;

        case MimeSubTypeXsgi:
            s = "X-sgi-movie";
            break;

        case MimeSubTypePNG:
            s = "PNG";
            break;

        case MimeSubTypeTIFF:
            s = "TIFF";
            break;

        case MimeSubTypeGL:
            s = "GL";
            break;

        case MimeSubTypeFLI:
            s = "FLI";
            break;

        case MimeSubTypeQuickTime:
            s = "quicktime";
            break;

        case MimeSubTypeXvrml:
            s = "X-VRML";
            break;

        case MimeSubTypeMIDI:
            s = "MIDI";
            break;

        case MimeSubTypeULAW:
            s = "ULAW";
            break;

        case MimeSubTypeXaiff:
            s = "X-AIFF";
            break;

        case MimeSubTypeXwav:
            s = "X-wav";
            break;

        case MimeSubTypeUnknown:
        default:
            s = "Unknown";
            break;
    }

    return s;
}

    CteType
RMM::RCteStr2Enum(const QCString & s)
{
    if (s.isEmpty()) return CteType7bit;
    CteType cte = CteType7bit;

    switch (s[0]) {

        case '7':

            if (stricmp(s, "7bit") == 0)
                cte = CteType7bit;

            break;

        case '8':

            if (stricmp(s, "8bit") == 0)
                cte = CteType8bit;

            break;

        case 'b':
        case 'B':

            if (stricmp(s, "binary") == 0)
                cte = CteTypeBinary;

            else if (stricmp(s, "base64") == 0)
                cte = CteTypeBase64;

            break;

        case 'q':
        case 'Q':

            if (stricmp(s, "quoted-printable") == 0)
                cte = CteTypeQuotedPrintable;

            break;

        default:
            cte = CteType7bit;
            break;
    }

    return cte;
}

    QCString
RMM::cteTypeEnumToStr(CteType t)
{
    QCString s;

    switch (t) {

        case CteType7bit:
            s = "7bit";
            break;

        case CteType8bit:
            s = "8bit";
            break;

        case CteTypeBinary:
            s = "binary";
            break;

        case CteTypeBase64:
            s = "base64";
            break;

        case CteTypeQuotedPrintable:
            s = "quoted-printable";
            break;

        default:
            s = "";
    }

    return s;
}

    QString
RMM::mimeTypeToIconName(MimeType t, MimeSubType st)
{
    QString s;
    
    switch (t) {

        case MimeTypeText:

            switch (st) {

            case MimeSubTypeNone:
                s = "unknown.png";
                break;

            case MimeSubTypePlain:
                s = "text.png";
                break;

            case MimeSubTypeRichtext:
                s = "richtext.png";
                break;

            case MimeSubTypeEnriched:
                s = "enriched.png";
                break;

            case MimeSubTypeHTML:
                s = "html.png";
                break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeTypeMultiPart:
    
            switch (st) {

                case MimeSubTypeMixed:
                    s = "mixed.png";
                    break;

                case MimeSubTypeAlternative:
                    s = "alternative.png";
                    break;

                case MimeSubTypeDigest:
                    s = "digest.png";
                    break;

                case MimeSubTypeParallel:
                    s = "parallel.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
                
            break;

        case MimeTypeMessage:
            
            switch (st) {

                case MimeSubTypeRFC822:
                    s = "rfc822.png";
                    break;

                case MimeSubTypePartial:
                    s = "partial.png";
                    break;

                case MimeSubTypeExternalBody:
                    s = "external-body.png";
                    break;

                default:
                    break;
            }

            break;
    
        case MimeTypeImage:

            switch (st) {
            
                case MimeSubTypeJpeg:
                    s = "jpeg.png";
                    break;

                case MimeSubTypeGIF:
                    s = "gif.png";
                    break;
        
                case MimeSubTypePNG:
                    s = "png.png";
                    break;

                case MimeSubTypeTIFF:
                    s = "tiff.png";
                    break;
                
                case MimeSubTypeXras:
                    s = "x-cmu-raster.png";
                    break;

                case MimeSubTypeXpnm:
                    s = "x-portable-anymap.png";
                    break;

                case MimeSubTypeXpbm:
                    s = "x-portable-bitmap.png";
                    break;

                case MimeSubTypeXpgm:
                    s = "x-portable-graymap.png";
                    break;

                case MimeSubTypeXppm:
                    s = "x-portable-pixmap.png";
                    break;

                case MimeSubTypeXrgb:
                    s = "x-rgb.png";
                    break;

                case MimeSubTypeXxbm:
                    s = "x-xbitmap.png";
                    break;

                case MimeSubTypeXxpm:
                    s = "x-xpixmap.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeTypeApplication:

            switch (st) {

                case MimeSubTypeXcpio:
                    s = "x-cpio.png";
                    break;

                case MimeSubTypeXdvi:
                    s = "x-dvi.png";
                    break;

                case MimeSubTypePostScript:
                    s = "postscript.png";
                    break;

                case MimeSubTypeOctetStream:
                    s = "octet-stream.png";
                    break;
                    
                case MimeSubTypeXperl:
                    s = "x-perl.png";
                    break;

                case MimeSubTypeXtar:
                    s = "x-tar.png";
                    break;

                case MimeSubTypeXdeb:
                    s = "x-deb.png";
                    break;

                case MimeSubTypeXrar:
                    s = "x-rar-compressed.png";
                    break;

                case MimeSubTypeXlatex:
                    s = "x-latex.png";
                    break;

                case MimeSubTypeXsh:
                    s = "x-sh.png";
                    break;

                case MimeSubTypeXshar:
                    s = "x-shar.png";
                    break;

                case MimeSubTypeXtgz:
                    s = "x-tar-gz.png";
                    break;

                case MimeSubTypeXtcl:
                    s = "x-tcl.png";
                    break;

                case MimeSubTypeXtex:
                    s = "x-tex.png";
                    break;

                case MimeSubTypeXtroff:
                    s = "x-troff.png";
                    break;

                case MimeSubTypeXzip:
                    s = "x-zip.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeTypeAudio:

            switch (st) {
            
                case MimeSubTypeBasic:
                    s = "basic.png";
                    break;

                case MimeSubTypeMIDI:
                    s = "midi.png";
                    break;

                case MimeSubTypeULAW:
                    s = "ulaw.png";
                    break;

                case MimeSubTypeXaiff:
                    s = "x-aiff.png";
                    break;

                case MimeSubTypeXwav:
                    s = "x-wav.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
    
            break;

        case MimeTypeVideo:

            switch (st) {
        
                case MimeSubTypeMPEG:
                    s = "mpeg.png";
                    break;
            
                case MimeSubTypeXavi:
                    s = "x-msvideo.png";
                    break;

                case MimeSubTypeXsgi:
                    s = "x-sgi-movie.png";
                    break;
                    
                case MimeSubTypeGL:
                    s = "gl.png";
                    break;

                case MimeSubTypeFLI:
                    s = "fli.png";
                    break;

                case MimeSubTypeQuickTime:
                    s = "quicktime.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeTypeModel:
            
            // This shouldn't be here, but it'll do for now.
            switch (st) {
            
                case MimeSubTypeXvrml:
                    s = "x-vrml.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
            
            break;

        case MimeTypeNone:
        case MimeTypeUnknown:
        default:
            s = "unknown.png";
            break;
    }
    
    return s;
}

    HeaderType
RMM::headerNameToEnum(const QCString & headerName)
{
    for (int i = 0; i < HeaderUnknown + 1; i++)
        if (0 == stricmp(headerName, headerNames[i]))
            return (HeaderType)i;

    return HeaderUnknown;
}

    HeaderDataType
RMM::headerNameToType(const QCString & headerName)
{
    HeaderType t(headerNameToEnum(headerName));
    
    return headerTypesTable[t];
    
    return Text;
}
// vim:ts=4:sw=4:tw=78
