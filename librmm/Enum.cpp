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

#include <rmm/Enum.h>

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


    const HeaderClass
RMM::headerTypesTable [] = {
    ClassText,                // HeaderApproved
    ClassAddressList,        // HeaderBcc
    ClassAddressList,        // HeaderCc
    ClassText,                // HeaderComments
    ClassText,                // HeaderContentDescription
    ClassContentDisposition,   // HeaderContentDisposition
    ClassMessageID,            // HeaderContentID
    ClassText,                // HeaderContentMD5
    ClassContentType,        // HeaderContentType
    ClassText,                // HeaderControl
    ClassCte,                // HeaderContentTransferEncoding
    ClassDateTime,            // HeaderDate
    ClassText,                // HeaderDistribution
    ClassText,                // HeaderEncrypted
    ClassDateTime,            // HeaderExpires
    ClassText,                // HeaderFollowupTo
    ClassAddressList,        // HeaderFrom
    ClassText,                // HeaderInReplyTo
    ClassText,                // HeaderKeywords
    ClassText,                // HeaderLines
    ClassMessageID,            // HeaderMessageID
    ClassText,                // HeaderMimeVersion
    ClassText,                // HeaderNewsgroups
    ClassText,                // HeaderOrganization
    ClassText,                // HeaderPath
    ClassText,                // HeaderReceived
    ClassText,                // HeaderReferences
    ClassAddressList,        // HeaderReplyTo
    ClassAddressList,        // HeaderResentBcc
    ClassAddressList,        // HeaderResentCc
    ClassDateTime,            // HeaderResentDate
    ClassAddressList,        // HeaderResentFrom
    ClassMessageID,            // HeaderResentMessageID
    ClassAddressList,        // HeaderResentReplyTo
    ClassAddress,            // HeaderResentSender
    ClassAddressList,        // HeaderResentTo
    ClassText,                // HeaderReturnPath
    ClassAddress,            // HeaderSender
    ClassText,                // HeaderSubject
    ClassText,                // HeaderSummary
    ClassAddressList,        // HeaderTo
    ClassText,                // HeaderXref
    ClassText                // HeaderUnknown
};

    Month
RMM::strToMonth(const QCString & s)
{
    if (s.isEmpty()) return MonthJan;

    for (int i = 1; i <= 12; i++)
        if (!qstricmp(s, monthNames[i-1])) return (Month)i;

    return MonthJan;

}

    MimeGroup
RMM::mimeGroupStr2Enum(const QCString & s)
{
    if (s.isEmpty()) return MimeGroupNone;

    MimeGroup t = MimeGroupUnknown;

    switch (s[0]) {

        case 0:

            t = MimeGroupNone;

        case 'a':
        case 'A':

            if (qstricmp(s, "application") == 0)
                t = MimeGroupApplication;

            else if (qstricmp(s, "audio") == 0)
                t = MimeGroupAudio;

            break;

        case 'i':
        case 'I':

            if (qstricmp(s, "image") == 0)
                t = MimeGroupImage;

            break;

        case 'm':
        case 'M':

            if (qstricmp(s, "message") == 0)
                t = MimeGroupMessage;

            else if (qstricmp(s, "multipart") == 0)
                t = MimeGroupMultiPart;

            break;

        case 't':
        case 'T':

            if (qstricmp(s, "text") == 0)
                t = MimeGroupText;

            break;

        case 'v':
        case 'V':

            if (qstricmp(s, "video") == 0)
                t = MimeGroupVideo;

            break;

        default:

            t = MimeGroupUnknown;
    }

    return t;
}

    QCString
RMM::mimeGroupEnum2Str(MimeGroup m)
{
    QCString s;

    switch (m) {

        case MimeGroupNone:
            s = "Unknown";
            break;

        case MimeGroupUnknown:
            s = "Unknown";
            break;

        case MimeGroupText:
            s = "Text";
            break;

        case MimeGroupMultiPart:
            s = "Multipart";
            break;

        case MimeGroupMessage:
            s = "Message";
            break;

        case MimeGroupImage:
            s = "Image";
            break;

        case MimeGroupApplication:
            s = "Application";
            break;

        case MimeGroupAudio:
            s = "Audio";
            break;

        case MimeGroupVideo:
            s = "Video";
            break;

        case MimeGroupModel:
            s = "Model";
            break;

        default:
            s = "Unknown";
            break;
    }

    return s;
}

    MimeValue
RMM::mimeValueStr2Enum(const QCString & s)
{
    if (s.isEmpty()) return MimeValueNone;

    MimeValue st = MimeValueNone;

    switch (s[0]) {

        case 'a':
        case 'A':

            if (qstricmp(s, "alternative") == 0)
                st = MimeValueAlternative;

            break;

        case 'b':
        case 'B':

            if (qstricmp(s, "basic") == 0)
                st = MimeValueBasic;

            break;

        case 'd':
        case 'D':

            if (qstricmp(s, "digest") == 0)
                st = MimeValueDigest;

            break;

        case 'e':
        case 'E':

            if (qstricmp(s, "enriched") == 0)
                st = MimeValueEnriched;

            else if (qstricmp(s, "external-body") == 0)
                st = MimeValueExternalBody;

            break;

        case 'g':
        case 'G':

            if (qstricmp(s, "gif") == 0)
                st = MimeValueGIF;

            break;

        case 'h':
        case 'H':

            if (qstricmp(s, "html") == 0)
                st = MimeValueHTML;

            break;

        case 'j':
        case 'J':

            if (qstricmp(s, "jpeg") == 0)
                st = MimeValueJpeg;

            break;

        case 'm':
        case 'M':

            if (qstricmp(s, "mixed") == 0)
                st = MimeValueMixed;

            else if (qstricmp(s, "mpeg") == 0)
                st = MimeValueMPEG;

            break;

        case 'o':
        case 'O':

            if (qstricmp(s, "octet-stream") == 0)
                st = MimeValueOctetStream;

            break;

        case 'p':
        case 'P':

            if (qstricmp(s, "plain") == 0)
                st = MimeValuePlain;

            else if (qstricmp(s, "png") == 0)
                st = MimeValuePNG;
            
            else if (qstricmp(s, "postscript") == 0)
                st = MimeValuePostScript;

            else if (qstricmp(s, "parallel") == 0)
                st = MimeValueParallel;

            else if (qstricmp(s, "partial") == 0)
                st = MimeValuePartial;

            break;

        case 'r':
        case 'R':

            if (qstricmp(s, "rfc822") == 0)
                st = MimeValueRFC822;

            else if (qstricmp(s, "richtext") == 0)
                st = MimeValueRichtext;

            break;
            
        case 'x':
        case 'X':

            switch (s[2]) {
                
                case 'A':
                case 'a':
                    
                    if (qstricmp(s, "x-msvideo") == 0)
                        st = MimeValueXavi;
            
                    else if (qstricmp(s, "x-aiff") == 0)
                        st = MimeValueXaiff;

                    break;
                
                case 'C':
                case 'c':
                
                    if (qstricmp(s, "x-cpio") == 0)
                        st = MimeValueXcpio;
                    break;

                case 'D':
                case 'd':
                    
                    if (qstricmp(s, "x-dvi") == 0)
                        st = MimeValueXdvi;
                    else if (qstricmp(s, "x-deb") == 0)
                        st = MimeValueXdeb;
                    break;

                case 'L':
                case 'l':
                
                    if (qstricmp(s, "x-latex") == 0)
                        st = MimeValueXlatex;
                    break;

                case 'P':
                case 'p':
                
                    if (qstricmp(s, "x-perl") == 0)
                        st = MimeValueXperl;

                    else if (qstricmp(s, "x-portable-anymap") == 0)
                        st = MimeValueXpnm;
                    
                    else if (qstricmp(s, "x-portable-bitmap") == 0)
                        st = MimeValueXpbm;
                    
                    else if (qstricmp(s, "x-portable-graymap") == 0)
                        st = MimeValueXpgm;
                    
                    else if (qstricmp(s, "x-portable-pixmap") == 0)
                        st = MimeValueXppm;

                    break;
                
                case 'R':
                case 'r':

                    if (qstricmp(s, "x-rar") == 0)
                        st = MimeValueXrar;

                    else if (qstricmp(s, "x-cmu-raster") == 0)
                        st = MimeValueXras;
            
                    else if (qstricmp(s, "x-rgb") == 0)
                        st = MimeValueXrgb;

                    break;

                case 'S':
                case 's':
    
                    if (qstricmp(s, "x-sh") == 0)
                        st = MimeValueXsh;

                    else if (qstricmp(s, "x-shar") == 0)
                        st = MimeValueXshar;
                    
                    else if (qstricmp(s, "x-sgi-movie") == 0)
                        st = MimeValueXsgi;

                    break;
    
                case 'T':
                case 't':
                    
                    if (qstricmp(s, "x-tar") == 0)
                        st = MimeValueXtar;

                    else if (qstricmp(s, "x-tgz") == 0)
                        st = MimeValueXtgz;

                    else if (qstricmp(s, "x-tcl") == 0)
                        st = MimeValueXtcl;

                    else if (qstricmp(s, "x-tex") == 0)
                        st = MimeValueXtex;

                    else if (qstricmp(s, "x-troff") == 0)
                        st = MimeValueXtroff;

                    break;
                    
                case 'X':
                case 'x':

                    if (qstricmp(s, "x-xbitmap") == 0)
                        st = MimeValueXxbm;
                    
                    else if (qstricmp(s, "x-xpixmap") == 0)
                        st = MimeValueXxpm;
                
                    break;
            
                case 'W':
                case 'w':
                    
                    if (qstricmp(s, "x-wav") == 0)
                        st = MimeValueXwav;
                    break;
                
                case 'Z':
                case 'z':

                    if (qstricmp(s, "x-zip") == 0)
                        st = MimeValueXzip;
                    break;
    
                    
                default:
                    break;
            }

            break;

        default:

            st = MimeValueUnknown;
            break;
    }

    return st;
}

    QCString
RMM::mimeValueEnum2Str(MimeValue t)
{
    QCString s;

    switch (t) {

        case MimeValueNone:
            s = "Unknown";
            break;


        case MimeValuePlain:
            s = "Plain";
            break;

        case MimeValueRichtext:
            s = "Richtext";
            break;

        case MimeValueEnriched:
            s = "Enriched";
            break;

        case MimeValueHTML:
            s = "HTML";
            break;

        case MimeValueMixed:
            s = "Mixed";
            break;

        case MimeValueAlternative:
            s = "Alternative";
            break;

        case MimeValueDigest:
            s = "Digest";
            break;

        case MimeValueParallel:
            s = "Parallel";
            break;

        case MimeValueRFC822:
            s = "RFC822";
            break;

        case MimeValuePartial:
            s = "Partial";
            break;

        case MimeValueExternalBody:
            s = "External-body";
            break;

        case MimeValuePostScript:
            s = "Postscript";
            break;

        case MimeValueOctetStream:
            s = "Octet-stream";
            break;

        case MimeValueJpeg:
            s = "JPEG";
            break;

        case MimeValueGIF:
            s = "GIF";
            break;

        case MimeValueBasic:
            s = "Basic";
            break;

        case MimeValueMPEG:
            s = "MPEG";
            break;
            
        case MimeValueXcpio:
            s = "X-cpio";
            break;

        case MimeValueXdvi:
            s = "X-dvi";
            break;

        case MimeValueXperl:
            s = "X-perl";
            break;

        case MimeValueXtar:
            s = "X-tar";
            break;

        case MimeValueXdeb:
            s = "X-deb";
            break;

        case MimeValueXrar:
            s = "X-rar-compressed";
            break;

        case MimeValueXlatex:
            s = "X-latex";
            break;

        case MimeValueXsh:
            s = "X-sh";
            break;

        case MimeValueXshar:
            s = "X-shar";
            break;

        case MimeValueXtgz:
            s = "X-tar-gz";
            break;

        case MimeValueXtcl:
            s = "X-tcl";
            break;

        case MimeValueXtex:
            s = "X-tex";
            break;

        case MimeValueXtroff:
            s = "X-troff";
            break;

        case MimeValueXzip:
            s = "X-zip";
            break;
            
        case MimeValueXras:
            s = "X-cmu-raster";
            break;

        case MimeValueXpnm:
            s = "X-portable-anymap";
            break;

        case MimeValueXpbm:
            s = "X-portable-bitmap";
            break;

        case MimeValueXpgm:
            s = "X-portable-graymap";
            break;

        case MimeValueXppm:
            s = "X-portable-pixmap";
            break;

        case MimeValueXrgb:
            s = "X-rgb";
            break;

        case MimeValueXxbm:
            s = "X-xbitmap";
            break;

        case MimeValueXxpm:
            s = "X-xpixmap";
            break;

        case MimeValueXavi:
            s = "X-msvideo";
            break;

        case MimeValueXsgi:
            s = "X-sgi-movie";
            break;

        case MimeValuePNG:
            s = "PNG";
            break;

        case MimeValueTIFF:
            s = "TIFF";
            break;

        case MimeValueGL:
            s = "GL";
            break;

        case MimeValueFLI:
            s = "FLI";
            break;

        case MimeValueQuickTime:
            s = "quicktime";
            break;

        case MimeValueXvrml:
            s = "X-VRML";
            break;

        case MimeValueMIDI:
            s = "MIDI";
            break;

        case MimeValueULAW:
            s = "ULAW";
            break;

        case MimeValueXaiff:
            s = "X-AIFF";
            break;

        case MimeValueXwav:
            s = "X-wav";
            break;

        case MimeValueUnknown:
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

            if (qstricmp(s, "7bit") == 0)
                cte = CteType7bit;

            break;

        case '8':

            if (qstricmp(s, "8bit") == 0)
                cte = CteType8bit;

            break;

        case 'b':
        case 'B':

            if (qstricmp(s, "binary") == 0)
                cte = CteTypeBinary;

            else if (qstricmp(s, "base64") == 0)
                cte = CteTypeBase64;

            break;

        case 'q':
        case 'Q':

            if (qstricmp(s, "quoted-printable") == 0)
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
RMM::mimeGroupToIconName(MimeGroup t, MimeValue st)
{
    QString s;
    
    switch (t) {

        case MimeGroupText:

            switch (st) {

            case MimeValueNone:
                s = "unknown.png";
                break;

            case MimeValuePlain:
                s = "text.png";
                break;

            case MimeValueRichtext:
                s = "richtext.png";
                break;

            case MimeValueEnriched:
                s = "enriched.png";
                break;

            case MimeValueHTML:
                s = "html.png";
                break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeGroupMultiPart:
    
            switch (st) {

                case MimeValueMixed:
                    s = "mixed.png";
                    break;

                case MimeValueAlternative:
                    s = "alternative.png";
                    break;

                case MimeValueDigest:
                    s = "digest.png";
                    break;

                case MimeValueParallel:
                    s = "parallel.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
                
            break;

        case MimeGroupMessage:
            
            switch (st) {

                case MimeValueRFC822:
                    s = "rfc822.png";
                    break;

                case MimeValuePartial:
                    s = "partial.png";
                    break;

                case MimeValueExternalBody:
                    s = "external-body.png";
                    break;

                default:
                    break;
            }

            break;
    
        case MimeGroupImage:

            switch (st) {
            
                case MimeValueJpeg:
                    s = "jpeg.png";
                    break;

                case MimeValueGIF:
                    s = "gif.png";
                    break;
        
                case MimeValuePNG:
                    s = "png.png";
                    break;

                case MimeValueTIFF:
                    s = "tiff.png";
                    break;
                
                case MimeValueXras:
                    s = "x-cmu-raster.png";
                    break;

                case MimeValueXpnm:
                    s = "x-portable-anymap.png";
                    break;

                case MimeValueXpbm:
                    s = "x-portable-bitmap.png";
                    break;

                case MimeValueXpgm:
                    s = "x-portable-graymap.png";
                    break;

                case MimeValueXppm:
                    s = "x-portable-pixmap.png";
                    break;

                case MimeValueXrgb:
                    s = "x-rgb.png";
                    break;

                case MimeValueXxbm:
                    s = "x-xbitmap.png";
                    break;

                case MimeValueXxpm:
                    s = "x-xpixmap.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeGroupApplication:

            switch (st) {

                case MimeValueXcpio:
                    s = "x-cpio.png";
                    break;

                case MimeValueXdvi:
                    s = "x-dvi.png";
                    break;

                case MimeValuePostScript:
                    s = "postscript.png";
                    break;

                case MimeValueOctetStream:
                    s = "octet-stream.png";
                    break;
                    
                case MimeValueXperl:
                    s = "x-perl.png";
                    break;

                case MimeValueXtar:
                    s = "x-tar.png";
                    break;

                case MimeValueXdeb:
                    s = "x-deb.png";
                    break;

                case MimeValueXrar:
                    s = "x-rar-compressed.png";
                    break;

                case MimeValueXlatex:
                    s = "x-latex.png";
                    break;

                case MimeValueXsh:
                    s = "x-sh.png";
                    break;

                case MimeValueXshar:
                    s = "x-shar.png";
                    break;

                case MimeValueXtgz:
                    s = "x-tar-gz.png";
                    break;

                case MimeValueXtcl:
                    s = "x-tcl.png";
                    break;

                case MimeValueXtex:
                    s = "x-tex.png";
                    break;

                case MimeValueXtroff:
                    s = "x-troff.png";
                    break;

                case MimeValueXzip:
                    s = "x-zip.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeGroupAudio:

            switch (st) {
            
                case MimeValueBasic:
                    s = "basic.png";
                    break;

                case MimeValueMIDI:
                    s = "midi.png";
                    break;

                case MimeValueULAW:
                    s = "ulaw.png";
                    break;

                case MimeValueXaiff:
                    s = "x-aiff.png";
                    break;

                case MimeValueXwav:
                    s = "x-wav.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
    
            break;

        case MimeGroupVideo:

            switch (st) {
        
                case MimeValueMPEG:
                    s = "mpeg.png";
                    break;
            
                case MimeValueXavi:
                    s = "x-msvideo.png";
                    break;

                case MimeValueXsgi:
                    s = "x-sgi-movie.png";
                    break;
                    
                case MimeValueGL:
                    s = "gl.png";
                    break;

                case MimeValueFLI:
                    s = "fli.png";
                    break;

                case MimeValueQuickTime:
                    s = "quicktime.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }

            break;

        case MimeGroupModel:
            
            // This shouldn't be here, but it'll do for now.
            switch (st) {
            
                case MimeValueXvrml:
                    s = "x-vrml.png";
                    break;

                default:
                s = "unknown.png";
                    break;
            }
            
            break;

        case MimeGroupNone:
        case MimeGroupUnknown:
        default:
            s = "unknown.png";
            break;
    }
    
    return s;
}

    HeaderType
RMM::headerNameToType(const QCString & headerName)
{
    for (int i = 0; i < HeaderUnknown + 1; i++)
        if (0 == qstricmp(headerName, headerNames[i]))
            return (HeaderType)i;

    return HeaderUnknown;
}

    HeaderClass
RMM::headerNameToClass(const QCString & headerName)
{
    HeaderType t(headerNameToType(headerName));
    
    if (HeaderUnknown == t)
        return ClassText;

    return headerTypesTable[t];
}

    QCString
RMM::headerTypeToName(HeaderType t)
{
    if (t > HeaderUnknown)
        return "";
    else
        return headerNames[t];
}

// vim:ts=4:sw=4:tw=78
