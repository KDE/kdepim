//=============================================================================
// File:       dw_mime.cpp
// Contents:   Various functions
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
// $Revision$
// $Date$
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
//
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <mimelib/debug.h>
#include <mimelib/string.h>
#include <mimelib/msgcmp.h>
#include <mimelib/enum.h>
#include <mimelib/utility.h>


void DwInitialize()
{
}


void DwFinalize()
{
}


int DwCteStrToEnum(const DwString& aStr)
{
    int cte = DwMime::kCteUnknown;
    int ch = aStr[0];
    switch (ch) {
    case '7':
        if( DwStrcasecmp(aStr, "7bit") == 0 ) {
            cte = DwMime::kCte7bit;
        }
        break;
    case '8':
        if (DwStrcasecmp(aStr, "8bit") == 0) {
            cte = DwMime::kCte8bit;
        }
        break;
    case 'B':
    case 'b':
        if (DwStrcasecmp(aStr, "base64") == 0) {
            cte = DwMime::kCteBase64;
        }
        else if (DwStrcasecmp(aStr, "binary") == 0) {
            cte = DwMime::kCteBinary;
        }
        break;
    case 'Q':
    case 'q':
        if (DwStrcasecmp(aStr, "quoted-printable") == 0) {
            cte = DwMime::kCteQuotedPrintable;
        }
        break;
    }
    return cte;
}


void DwCteEnumToStr(int aEnum, DwString& aStr)
{
    switch (aEnum) {
    case DwMime::kCte7bit:
        aStr = "7bit";
        break;
    case DwMime::kCte8bit:
        aStr = "8bit";
        break;
    case DwMime::kCteBinary:
        aStr = "binary";
        break;
    case DwMime::kCteBase64:
        aStr = "base64";
        break;
    case DwMime::kCteQuotedPrintable:
        aStr = "quoted-printable";
        break;
    }
}


int DwTypeStrToEnum(const DwString& aStr)
{
    int type = DwMime::kTypeUnknown;
    int ch = aStr[0];
    switch (ch) {
    case 'A':
    case 'a':
        if (DwStrcasecmp(aStr, "application") == 0) {
            type = DwMime::kTypeApplication;
        }
        else if (DwStrcasecmp(aStr, "audio") == 0) {
            type = DwMime::kTypeAudio;
        }
        break;
    case 'I':
    case 'i':
        if (DwStrcasecmp(aStr, "image") == 0) {
            type = DwMime::kTypeImage;
        }
        break;
    case 'M':
    case 'm':
        if (DwStrcasecmp(aStr, "message") == 0) {
            type = DwMime::kTypeMessage;
        }
        else if (DwStrcasecmp(aStr, "multipart") == 0) {
            type = DwMime::kTypeMultipart;
        }
        break;
    case 'T':
    case 't':
        if (DwStrcasecmp(aStr, "text") == 0) {
            type = DwMime::kTypeText;
        }
        break;
    case 'V':
    case 'v':
        if (DwStrcasecmp(aStr, "video") == 0) {
            type = DwMime::kTypeVideo;
        }
        break;
    case 0:
        type = DwMime::kTypeNull;
        break;
    }
    return type;
}


void DwTypeEnumToStr(int aEnum, DwString& aStr)
{
    switch (aEnum) {
    case DwMime::kTypeNull:
        aStr = "";
        break;
    case DwMime::kTypeUnknown:
    default:
        aStr = "Unknown";
        break;
    case DwMime::kTypeText:
        aStr = "Text";
        break;
    case DwMime::kTypeMultipart:
        aStr = "Multipart";
        break;
    case DwMime::kTypeMessage:
        aStr = "Message";
        break;
    case DwMime::kTypeApplication:
        aStr = "Application";
        break;
    case DwMime::kTypeImage:
        aStr = "Image";
        break;
    case DwMime::kTypeAudio:
        aStr = "Audio";
        break;
    case DwMime::kTypeVideo:
        aStr = "Video";
        break;
    case DwMime::kTypeModel:
        aStr = "Model";
        break;
    }
}


int DwSubtypeStrToEnum(const DwString& aStr)
{
    if (aStr.empty()) {
        return DwMime::kSubtypeNull;
    }
    int type = DwMime::kSubtypeUnknown;
    int ch = aStr[0];
    switch (ch) {
    case 'A':
    case 'a':
        if (DwStrcasecmp(aStr, "alternative") == 0) {
            type = DwMime::kSubtypeAlternative;
        }
        break;
    case 'B':
    case 'b':
        if (DwStrcasecmp(aStr, "basic") == 0) {
            type = DwMime::kSubtypeBasic;
        }
        break;
    case 'C':
    case 'c':
        if (DwStrcasecmp(aStr, "calendar") == 0) {
            type = DwMime::kSubtypeVCal;
        }
        break;
    case 'D':
    case 'd':
        if (DwStrcasecmp(aStr, "digest") == 0) {
            type = DwMime::kSubtypeDigest;
        }
	else if (DwStrcasecmp(aStr, "disposition-notification") == 0 ) {
	    type = DwMime::kSubtypeDispositionNotification;
	}
        break;
    case 'E':
    case 'e':
        if (DwStrcasecmp(aStr, "enriched") == 0) {
            type = DwMime::kSubtypeEnriched;
        }
        else if (DwStrcasecmp(aStr, "external-body") == 0) {
            type = DwMime::kSubtypeExternalBody;
        }
        else if (DwStrcasecmp(aStr, "encrypted") == 0) {
            type = DwMime::kSubtypeEncrypted;
        }
        break;
    case 'G':
    case 'g':
        if (DwStrcasecmp(aStr, "gif") == 0) {
            type = DwMime::kSubtypeGif;
        }
        break;
    case 'H':
    case 'h':
        if (DwStrcasecmp(aStr, "html") == 0) {
            type = DwMime::kSubtypeHtml;
        }
        break;
    case 'J':
    case 'j':
        if (DwStrcasecmp(aStr, "jpeg") == 0) {
            type = DwMime::kSubtypeJpeg;
        }
        break;
    case 'M':
    case 'm':
        if (DwStrcasecmp(aStr, "mixed") == 0) {
            type = DwMime::kSubtypeMixed;
        }
        else if (DwStrcasecmp(aStr, "mpeg") == 0) {
            type = DwMime::kSubtypeMpeg;
        }
        else if (DwStrcasecmp(aStr, "ms-tnef") == 0) {
            type = DwMime::kSubtypeMsTNEF;
        }
        break;
    case 'O':
    case 'o':
        if (DwStrcasecmp(aStr, "octet-stream") == 0) {
            type = DwMime::kSubtypeOctetStream;
        }
        break;
    case 'P':
    case 'p':
        if (DwStrcasecmp(aStr, "plain") == 0) {
            type = DwMime::kSubtypePlain;
        }
        else if (DwStrcasecmp(aStr, "parallel") == 0) {
            type = DwMime::kSubtypeParallel;
        }
        else if (DwStrcasecmp(aStr, "partial") == 0) {
            type = DwMime::kSubtypePartial;
        }
        else if (DwStrcasecmp(aStr, "postscript") == 0) {
            type = DwMime::kSubtypePostscript;
        }
        else if (DwStrcasecmp(aStr, "pgp-signature") == 0) {
            type = DwMime::kSubtypePgpSignature;
        }
        else if (DwStrcasecmp(aStr, "pgp-encrypted") == 0) {
            type = DwMime::kSubtypePgpEncrypted;
        }
        else if (DwStrcasecmp(aStr, "pgp") == 0) {
            type = DwMime::kSubtypePgpClearsigned;
        }
        else if (DwStrcasecmp(aStr, "pkcs7-signature") == 0) {
            type = DwMime::kSubtypePkcs7Signature;
        }
        else if (DwStrcasecmp(aStr, "pkcs7-mime") == 0) {
            type = DwMime::kSubtypePkcs7Mime;
        }
        break;
    case 'R':
    case 'r':
        if (DwStrcasecmp(aStr, "richtext") == 0) {
            type = DwMime::kSubtypeRichtext;
        }
        else if (DwStrcasecmp(aStr, "rfc822") == 0) {
            type = DwMime::kSubtypeRfc822;
        }
	else if (DwStrcasecmp(aStr, "report") == 0) {
	    type = DwMime::kSubtypeReport;
	}
        else if (DwStrcasecmp(aStr, "rtf") == 0) {
            type = DwMime::kSubtypeRtf;
        }
        else if (DwStrcasecmp(aStr, "related") == 0) {
            type = DwMime::kSubtypeRelated;
        }
        break;
    case 'S':
    case 's':
        if (DwStrcasecmp(aStr, "signed") == 0) {
            type = DwMime::kSubtypeSigned;
        }
        break;
    case 'X':
    case 'x':
        if (DwStrcasecmp(aStr, "x-vcard") == 0) {
            type = DwMime::kSubtypeXVCard;
        }
        else if (DwStrcasecmp(aStr, "x-pkcs7-signature") == 0) {
            type = DwMime::kSubtypePkcs7Signature;
        }
        else if (DwStrcasecmp(aStr, "x-pkcs7-mime") == 0) {
            type = DwMime::kSubtypePkcs7Mime;
        }
        if (DwStrcasecmp(aStr, "x-diff") == 0) {
            type = DwMime::kSubtypeXDiff;
        }
        break;
    }
    return type;
}


void DwSubtypeEnumToStr(int aEnum, DwString& aStr)
{
    switch (aEnum) {
    case DwMime::kSubtypeNull:
        aStr = "";
        break;
    case DwMime::kSubtypeUnknown:
    default:
        aStr = "Unknown";
        break;
    case DwMime::kSubtypePlain:
        aStr = "Plain";
        break;
    case DwMime::kSubtypeRichtext:
        aStr = "Richtext";
        break;
    case DwMime::kSubtypeEnriched:
        aStr = "Enriched";
        break;
    case DwMime::kSubtypeHtml:
        aStr = "HTML";
        break;
    case DwMime::kSubtypeVCal:
        aStr = "Calendar";
        break;
    case DwMime::kSubtypeXVCard:
        aStr = "X-VCard";
        break;
    case DwMime::kSubtypeXDiff:
        aStr = "X-Diff";
        break;
    case DwMime::kSubtypeRtf:
        aStr = "RTF";
        break;
    case DwMime::kSubtypeMixed:
        aStr = "Mixed";
        break;
    case DwMime::kSubtypeSigned:
        aStr = "Signed";
        break;
    case DwMime::kSubtypeEncrypted:
        aStr = "Encrypted";
        break;
    case DwMime::kSubtypeAlternative:
        aStr = "Alternative";
        break;
    case DwMime::kSubtypeDigest:
        aStr = "Digest";
        break;
    case DwMime::kSubtypeParallel:
        aStr = "Parallel";
        break;
    case DwMime::kSubtypeReport:
        aStr = "report";
        break;
    case DwMime::kSubtypeRelated:
        aStr = "Related";
        break;
    case DwMime::kSubtypeRfc822:
        aStr = "Rfc822";
        break;
    case DwMime::kSubtypeDispositionNotification:
        aStr = "disposition-notification";
        break;
    case DwMime::kSubtypePartial:
        aStr = "Partial";
        break;
    case DwMime::kSubtypeExternalBody:
        aStr = "External-body";
        break;
    case DwMime::kSubtypePostscript:
        aStr = "Postscript";
        break;
    case DwMime::kSubtypeOctetStream:
        aStr = "Octet-stream";
        break;
    case DwMime::kSubtypePgpSignature:
        aStr = "pgp-signature";
        break;
    case DwMime::kSubtypePgpEncrypted:
        aStr = "pgp-encrypted";
        break;
    case DwMime::kSubtypePgpClearsigned:
        aStr = "pgp";
        break;
    case DwMime::kSubtypePkcs7Signature:
        aStr = "pkcs7-signature";
        break;
    case DwMime::kSubtypePkcs7Mime:
        aStr = "pkcs7-mime";
        break;
    case DwMime::kSubtypeMsTNEF:
        aStr = "ms-tnef";
        break;
    case DwMime::kSubtypeJpeg:
        aStr = "jpeg";
        break;
    case DwMime::kSubtypeGif:
        aStr = "gif";
        break;
    case DwMime::kSubtypeBasic:
        aStr = "basic";
        break;
    case DwMime::kSubtypeMpeg:
        aStr = "mpeg";
        break;
    }
}

