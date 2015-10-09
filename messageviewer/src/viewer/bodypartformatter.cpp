/*  -*- c++ -*-
    bodypartformatter.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "bodypartformatter.h"
#include "messageviewer_debug.h"
#include "viewer/bodypartformatterfactory_p.h"
#include "viewer/attachmentstrategy.h"
#include "interfaces/bodypartformatter.h"
#include "interfaces/bodypart.h"

#include "viewer/objecttreeparser.h"
#include "messagepart.h"

#include <kmime/kmime_content.h>

using namespace MessageViewer;

namespace
{
class AnyTypeBodyPartFormatter
    : public BodyPartFormatter,
      public MessageViewer::Interface::BodyPartFormatter
{
    static const AnyTypeBodyPartFormatter *self;
public:
    Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE
    {
        qCDebug(MESSAGEVIEWER_LOG) << "Acting as a Interface::BodyPartFormatter!";
        return AsIcon;
    }

    // unhide the overload with three arguments
    using MessageViewer::Interface::BodyPartFormatter::format;

    bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &result) const Q_DECL_OVERRIDE
    {
        result.setNeverDisplayInline(true);
        return false;
    }
    static const ::BodyPartFormatter *create()
    {
        if (!self) {
            self = new AnyTypeBodyPartFormatter();
        }
        return self;
    }
};

const AnyTypeBodyPartFormatter *AnyTypeBodyPartFormatter::self = 0;

class ImageTypeBodyPartFormatter
    : public BodyPartFormatter
    , public MessageViewer::Interface::BodyPartFormatter
{
    static const ImageTypeBodyPartFormatter *self;
public:
    Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE
    {
        return AsIcon;
    }

    // unhide the overload with three arguments
    using MessageViewer::Interface::BodyPartFormatter::format;

    bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &result) const Q_DECL_OVERRIDE
    {
        result.setIsImage(true);
        return false;
    }
    static const ::BodyPartFormatter *create()
    {
        if (!self) {
            self = new ImageTypeBodyPartFormatter();
        }
        return self;
    }
};

const ImageTypeBodyPartFormatter *ImageTypeBodyPartFormatter::self = 0;

class MessageRfc822BodyPartFormatter
    : public BodyPartFormatter
    , public MessageViewer::Interface::BodyPartFormatter
{
    static const MessageRfc822BodyPartFormatter * self;
public:
    bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &) const Q_DECL_OVERRIDE;
    MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE;
    using MessageViewer::Interface::BodyPartFormatter::format;
    static const ::BodyPartFormatter *create();
};

const MessageRfc822BodyPartFormatter *MessageRfc822BodyPartFormatter::self;

const ::BodyPartFormatter *MessageRfc822BodyPartFormatter::create() {
    if (!self) {
        self = new MessageRfc822BodyPartFormatter();
    }
    return self;
}

bool MessageRfc822BodyPartFormatter::process(ObjectTreeParser * otp, KMime::Content * node, ProcessResult & result) const
{
    PartMetaData metaData;
    const KMime::Message::Ptr message = node->bodyAsMessage();
    EncapsulatedRfc822MessagePart mp(otp, &metaData, node, message);

    if (!otp->attachmentStrategy()->inlineNestedMessages() && !otp->showOnlyOneMimePart()) {
        return false;
    } else {
        mp.html(true);
        return true;
    }
}

MessageViewer::Interface::BodyPartFormatter::Result MessageRfc822BodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const
{
    bool ret = process(part->objectTreeParser(), part->content(), *part->processResult());
    return ret ? Ok: Failed;
}

#define CREATE_BODY_PART_FORMATTER(subtype) \
    class subtype##BodyPartFormatter \
        : public BodyPartFormatter \
        , public MessageViewer::Interface::BodyPartFormatter \
        { \
        static const subtype##BodyPartFormatter * self; \
    public: \
        bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &) const Q_DECL_OVERRIDE;\
        MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE; \
        using MessageViewer::Interface::BodyPartFormatter::format; \
        static const ::BodyPartFormatter *create(); \
    }; \
    \
    const subtype##BodyPartFormatter *subtype##BodyPartFormatter::self; \
    \
    const ::BodyPartFormatter *subtype##BodyPartFormatter::create() { \
        if ( !self ) { \
            self = new subtype##BodyPartFormatter(); \
        } \
        return self; \
    } \
    bool subtype##BodyPartFormatter::process(ObjectTreeParser * otp, KMime::Content * node, ProcessResult & result) const { \
        return otp->process##subtype##Subtype(node, result); \
    } \
    \
    MessageViewer::Interface::BodyPartFormatter::Result subtype##BodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const { \
        const bool result = process(part->objectTreeParser(), part->content(), *part->processResult()); \
        return result ? Ok: Failed; \
    }

CREATE_BODY_PART_FORMATTER(TextPlain)
CREATE_BODY_PART_FORMATTER(TextHtml)
//CREATE_BODY_PART_FORMATTER(TextEnriched)

CREATE_BODY_PART_FORMATTER(ApplicationPkcs7Mime)
CREATE_BODY_PART_FORMATTER(ApplicationChiasmusText)
//CREATE_BODY_PART_FORMATTER(ApplicationPgp)

CREATE_BODY_PART_FORMATTER(MultiPartMixed)
CREATE_BODY_PART_FORMATTER(MultiPartAlternative)
CREATE_BODY_PART_FORMATTER(MultiPartSigned)
CREATE_BODY_PART_FORMATTER(MultiPartEncrypted)

typedef TextPlainBodyPartFormatter ApplicationPgpBodyPartFormatter;

#undef CREATE_BODY_PART_FORMATTER
} // anon namespace

// FIXME: port some more BodyPartFormatters to Interface::BodyPartFormatters
void BodyPartFormatterFactoryPrivate::messageviewer_create_builtin_bodypart_formatters(BodyPartFormatterFactoryPrivate::TypeRegistry *reg)
{
    if (!reg) {
        return;
    }
    (*reg)["application"]["octet-stream"] = new AnyTypeBodyPartFormatter;
    (*reg)["application"]["pgp"] = new ApplicationPgpBodyPartFormatter;
    (*reg)["application"]["pkcs7-mime"] = new ApplicationPkcs7MimeBodyPartFormatter;
    (*reg)["application"]["x-pkcs7-mime"] = new ApplicationPkcs7MimeBodyPartFormatter;
    (*reg)["application"]["vnd.de.bund.bsi.chiasmus-text"] = new ApplicationChiasmusTextBodyPartFormatter;
    (*reg)["application"]["*"] = new AnyTypeBodyPartFormatter;

    (*reg)["text"]["plain"] = new TextPlainBodyPartFormatter;
    (*reg)["text"]["html"] = new TextHtmlBodyPartFormatter;
    (*reg)["text"]["rtf"] = new AnyTypeBodyPartFormatter;
    (*reg)["text"]["vcard"] = new AnyTypeBodyPartFormatter;
    (*reg)["text"]["x-vcard"] = new AnyTypeBodyPartFormatter;
    (*reg)["text"]["*"] = new TextPlainBodyPartFormatter;

    (*reg)["image"]["*"] = new ImageTypeBodyPartFormatter;

    (*reg)["message"]["rfc822"] = new MessageRfc822BodyPartFormatter;
    (*reg)["message"]["*"] = new AnyTypeBodyPartFormatter;

    (*reg)["multipart"]["alternative"] = new MultiPartAlternativeBodyPartFormatter;
    (*reg)["multipart"]["encrypted"] = new MultiPartEncryptedBodyPartFormatter;
    (*reg)["multipart"]["signed"] = new MultiPartSignedBodyPartFormatter;
    (*reg)["multipart"]["*"] = new MultiPartMixedBodyPartFormatter;
}

typedef const BodyPartFormatter *(*BodyPartFormatterCreator)();

struct SubtypeBuiltin {
    const char *subtype;
    BodyPartFormatterCreator create;
};

static const BodyPartFormatter *createForText(const char *subtype)
{
    if (subtype && *subtype)
        switch (subtype[0]) {
        case 'h':
        case 'H':
            if (qstricmp(subtype, "html") == 0) {
                return TextHtmlBodyPartFormatter::create();
            }
            break;
        case 'r':
        case 'R':
            if (qstricmp(subtype, "rtf") == 0) {
                return AnyTypeBodyPartFormatter::create();
            }
            break;
        case 'x':
        case 'X':
        case 'v':
        case 'V':
            if (qstricmp(subtype, "x-vcard") == 0 ||
                    qstricmp(subtype, "vcard") == 0) {
                return AnyTypeBodyPartFormatter::create();
            }
            break;
        }

    return TextPlainBodyPartFormatter::create();
}

static const BodyPartFormatter *createForImage(const char *)
{
    return ImageTypeBodyPartFormatter::create();
}

static const BodyPartFormatter *createForMessage(const char *subtype)
{
    if (qstricmp(subtype, "rfc822") == 0) {
        return MessageRfc822BodyPartFormatter::create();
    }
    return AnyTypeBodyPartFormatter::create();
}

static const BodyPartFormatter *createForMultiPart(const char *subtype)
{
    if (subtype && *subtype)
        switch (subtype[0]) {
        case 'a':
        case 'A':
            if (qstricmp(subtype, "alternative") == 0) {
                return MultiPartAlternativeBodyPartFormatter::create();
            }
            break;
        case 'e':
        case 'E':
            if (qstricmp(subtype, "encrypted") == 0) {
                return MultiPartEncryptedBodyPartFormatter::create();
            }
            break;
        case 's':
        case 'S':
            if (qstricmp(subtype, "signed") == 0) {
                return MultiPartSignedBodyPartFormatter::create();
            }
            break;
        }

    return MultiPartMixedBodyPartFormatter::create();
}

static const BodyPartFormatter *createForApplication(const char *subtype)
{
    if (subtype && *subtype)
        switch (subtype[0]) {
        case 'p':
        case 'P':
            if (qstricmp(subtype, "pgp") == 0) {
                return ApplicationPgpBodyPartFormatter::create();
            }
        // fall through
        case 'x':
        case 'X':
            if (qstricmp(subtype, "pkcs7-mime") == 0 ||
                    qstricmp(subtype, "x-pkcs7-mime") == 0) {
                return ApplicationPkcs7MimeBodyPartFormatter::create();
            }
            break;
        case 'v':
        case 'V':
            if (qstricmp(subtype, "vnd.de.bund.bsi.chiasmus-text") == 0) {
                return ApplicationChiasmusTextBodyPartFormatter::create();
            }
            break;
        }

    return AnyTypeBodyPartFormatter::create();
}

// OK, replace this with a factory with plugin support later on...
const BodyPartFormatter *BodyPartFormatter::createFor(const char *type, const char *subtype)
{
    if (type && *type)
        switch (type[0]) {
        case 'a': // application
        case 'A':
            if (qstricmp(type, "application") == 0) {
                return createForApplication(subtype);
            }
            break;
        case 'i': // image
        case 'I':
            if (qstricmp(type, "image") == 0) {
                return createForImage(subtype);
            }
            break;
        case 'm': // multipart / message
        case 'M':
            if (qstricmp(type, "multipart") == 0) {
                return createForMultiPart(subtype);
            } else if (qstricmp(type, "message") == 0) {
                return createForMessage(subtype);
            }
            break;
        case 't': // text
        case 'T':
            if (qstricmp(type, "text") == 0) {
                return createForText(subtype);
            }
            break;
        }

    return AnyTypeBodyPartFormatter::create();
}
