//=============================================================================
// File:       enum.h
// Contents:   Declarations of global constants and function prototypes
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

#ifndef DW_ENUM_H
#define DW_ENUM_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

//-----------------------------------------------------------------------------
// Enumerated values
//-----------------------------------------------------------------------------

#if defined(DW_USE_NAMESPACES)
namespace DwMime {
#else
struct DwMime {
#endif

// Content transfer encoding

enum {
    kCteNull,
    kCteUnknown,
    kCte7bit,
    kCte8bit,
    kCteBinary,
    kCteQuotedPrintable,
    kCteQp = kCteQuotedPrintable,
    kCteBase64,
    kCteLast
};

// Content types

enum {
    kTypeNull,
    kTypeUnknown,
    kTypeText,
    kTypeMultipart,
    kTypeMessage,
    kTypeApplication,
    kTypeImage,
    kTypeAudio,
    kTypeVideo,
    kTypeModel,
    kTypeLast
};

// Content subtypes

enum {
    kSubtypeNull,
    kSubtypeUnknown,
    // Text
    kSubtypePlain,    // RFC-1521
    kSubtypeRichtext, // RFC-1341
    kSubtypeEnriched,
    kSubtypeHtml,
    kSubtypeXVCard,
    kSubtypeVCal,
    kSubtypeRtf,
    kSubtypeXDiff,
    // Multipart
    kSubtypeMixed,
    kSubtypeAlternative,
    kSubtypeDigest,
    kSubtypeParallel,
    kSubtypeSigned,
    kSubtypeEncrypted,
    kSubtypeReport,
    kSubtypeRelated,
    // Message
    kSubtypeRfc822,
    kSubtypeDispositionNotification,
    // Signed content
    kSubtypePartial,
    kSubtypeExternalBody,
    // Application
    kSubtypePostscript,
    kSubtypeOctetStream,
    kSubtypePgpSignature,
    kSubtypePgpEncrypted,
    kSubtypePgpClearsigned,
    kSubtypePkcs7Signature,
    kSubtypePkcs7Mime,
    kSubtypeMsTNEF,
    // Image
    kSubtypeJpeg,
    kSubtypeGif,
    // Audio
    kSubtypeBasic,
    // Video
    kSubtypeMpeg,
    // Last
    kSubtypeLast
};

// Well-known header fields

enum {
    kFldNull,
    kFldUnknown,
    // RFC-822
    kFldBcc,
    kFldCc,
    kFldComments,
    kFldDate,
    kFldEncrypted,
    kFldFrom,
    kFldInReplyTo,
    kFldKeywords,
    kFldMessageId,
    kFldReceived,
    kFldReferences,
    kFldReplyTo,
    kFldResentBcc,
    kFldResentCc,
    kFldResentDate,
    kFldResentFrom,
    kFldResentMessageId,
    kFldResentReplyTo,
    kFldResentSender,
    kFldResentTo,
    kFldReturnPath,
    kFldSender,
    kFldTo,
    kFldSubject,
    // RFC-1036
    kFldApproved,
    kFldControl,
    kFldDistribution,
    kFldExpires,
    kFldFollowupTo,
    kFldLines,
    kFldNewsgroups,
    kFldOrganization,
    kFldPath,
    kFldSummary,
    kFldXref,
    // RFC-1521
    kFldContentDescription,
    kFldContentId,
    kFldContentTransferEncoding,
    kFldCte = kFldContentTransferEncoding,
    kFldContentType,
    kFldMimeVersion,
    // RFC-1544
    kFldContentMd5,
    // RFC-1806
    kFldContentDisposition,
    // Last
    kFldLast
};


// Disposition type (Content-Disposition header field, see RFC-1806)
enum {
    kDispTypeNull,
    kDispTypeUnknown,
    kDispTypeInline,
    kDispTypeAttachment
};


#if defined(DW_USE_NAMESPACES)
}  // end namespace DwMime
#else
}; // end DwMime class declaration
#endif

#endif
