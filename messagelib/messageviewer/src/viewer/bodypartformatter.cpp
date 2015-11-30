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
    : public MessageViewer::Interface::BodyPartFormatter
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

    bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &result) const
    {
        result.setNeverDisplayInline(true);
        return false;
    }
    static const MessageViewer::Interface::BodyPartFormatter *create()
    {
        if (!self) {
            self = new AnyTypeBodyPartFormatter();
        }
        return self;
    }
};

const AnyTypeBodyPartFormatter *AnyTypeBodyPartFormatter::self = 0;

class ImageTypeBodyPartFormatter
    : public MessageViewer::Interface::BodyPartFormatter
{
    static const ImageTypeBodyPartFormatter *self;
public:
    Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE
    {
        return AsIcon;
    }

    // unhide the overload with three arguments
    using MessageViewer::Interface::BodyPartFormatter::format;

    bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &result) const
    {
        result.setIsImage(true);
        return false;
    }
    static const MessageViewer::Interface::BodyPartFormatter *create()
    {
        if (!self) {
            self = new ImageTypeBodyPartFormatter();
        }
        return self;
    }
};

const ImageTypeBodyPartFormatter *ImageTypeBodyPartFormatter::self = 0;

class MessageRfc822BodyPartFormatter
    : public MessageViewer::Interface::BodyPartFormatter
{
    static const MessageRfc822BodyPartFormatter *self;
public:
    MessagePart::Ptr process(Interface::BodyPart *) const;
    MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE;
    using MessageViewer::Interface::BodyPartFormatter::format;
    static const MessageViewer::Interface::BodyPartFormatter *create();
};

const MessageRfc822BodyPartFormatter *MessageRfc822BodyPartFormatter::self;

const MessageViewer::Interface::BodyPartFormatter *MessageRfc822BodyPartFormatter::create()
{
    if (!self) {
        self = new MessageRfc822BodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr MessageRfc822BodyPartFormatter::process(Interface::BodyPart *part) const
{
    const KMime::Message::Ptr message = part->content()->bodyAsMessage();
    return MessagePart::Ptr(new EncapsulatedRfc822MessagePart(part->objectTreeParser(), part->content(), message));
}

MessageViewer::Interface::BodyPartFormatter::Result MessageRfc822BodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const
{
    Q_UNUSED(writer)
    const ObjectTreeParser *otp = part->objectTreeParser();
    const MessagePart::Ptr mp = process(part);
    if (mp && !otp->attachmentStrategy()->inlineNestedMessages() && !otp->showOnlyOneMimePart()) {
        return Failed;
    } else {
        mp->html(true);
        return Ok;
    }
}

class MultiPartSignedBodyPartFormatter
    : public MessageViewer::Interface::BodyPartFormatter
{
public:
    MessagePart::Ptr process(const Interface::BodyPart &part) const;
    MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE;
    using MessageViewer::Interface::BodyPartFormatter::format;
    static const MessageViewer::Interface::BodyPartFormatter *create();
private:
    static const MultiPartSignedBodyPartFormatter *self;
};

const MultiPartSignedBodyPartFormatter *MultiPartSignedBodyPartFormatter::self;

const MessageViewer::Interface::BodyPartFormatter *MultiPartSignedBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartSignedBodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr MultiPartSignedBodyPartFormatter::process(const Interface::BodyPart &part) const
{
    return part.objectTreeParser()->processMultiPartSignedSubtype(part.content(), *part.processResult());
}

MessageViewer::Interface::BodyPartFormatter::Result MultiPartSignedBodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const
{
    Q_UNUSED(writer)
    MessagePart::Ptr mp = process(*part);
    if (!mp.isNull()) {
        mp->html(false);
        return Ok;
    }

    return Failed;
}

class MultiPartEncryptedBodyPartFormatter
    : public MessageViewer::Interface::BodyPartFormatter
{
public:
    MessagePart::Ptr process(const Interface::BodyPart &part) const;
    MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE;
    using MessageViewer::Interface::BodyPartFormatter::format;
    static const MessageViewer::Interface::BodyPartFormatter *create();
private:
    static const MultiPartEncryptedBodyPartFormatter *self;
};

const MultiPartEncryptedBodyPartFormatter *MultiPartEncryptedBodyPartFormatter::self;

const MessageViewer::Interface::BodyPartFormatter *MultiPartEncryptedBodyPartFormatter::create()
{
    if (!self) {
        self = new MultiPartEncryptedBodyPartFormatter();
    }
    return self;
}

MessagePart::Ptr MultiPartEncryptedBodyPartFormatter::process(const Interface::BodyPart &part) const
{
    return part.objectTreeParser()->processMultiPartEncryptedSubtype(part.content(), *part.processResult());
}

MessageViewer::Interface::BodyPartFormatter::Result MultiPartEncryptedBodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const
{
    Q_UNUSED(writer)
    MessagePart::Ptr mp = process(*part);
    if (!mp.isNull()) {
        mp->html(false);
        return Ok;
    }

    return Failed;
}

#define CREATE_BODY_PART_FORMATTER(subtype) \
    class subtype##BodyPartFormatter \
        : public MessageViewer::Interface::BodyPartFormatter \
    { \
        static const subtype##BodyPartFormatter *self; \
    public: \
        bool process(ObjectTreeParser *, KMime::Content *, ProcessResult &) const;\
        MessageViewer::Interface::BodyPartFormatter::Result format(Interface::BodyPart *, HtmlWriter *) const Q_DECL_OVERRIDE; \
        using MessageViewer::Interface::BodyPartFormatter::format; \
        static const MessageViewer::Interface::BodyPartFormatter *create(); \
    }; \
    \
    const subtype##BodyPartFormatter *subtype##BodyPartFormatter::self; \
    \
    const MessageViewer::Interface::BodyPartFormatter *subtype##BodyPartFormatter::create() { \
        if ( !self ) { \
            self = new subtype##BodyPartFormatter(); \
        } \
        return self; \
    } \
    bool subtype##BodyPartFormatter::process(ObjectTreeParser *otp, KMime::Content *node, ProcessResult &result) const { \
        return otp->process##subtype##Subtype(node, result); \
    } \
    \
    MessageViewer::Interface::BodyPartFormatter::Result subtype##BodyPartFormatter::format(Interface::BodyPart *part, HtmlWriter *writer) const { \
        Q_UNUSED(writer) \
        const bool result = process(part->objectTreeParser(), part->content(), *part->processResult()); \
        return result ? Ok : Failed; \
    }

CREATE_BODY_PART_FORMATTER(TextPlain)
CREATE_BODY_PART_FORMATTER(TextHtml)

CREATE_BODY_PART_FORMATTER(ApplicationPkcs7Mime)

CREATE_BODY_PART_FORMATTER(MultiPartMixed)
CREATE_BODY_PART_FORMATTER(MultiPartAlternative)

typedef TextPlainBodyPartFormatter ApplicationPgpBodyPartFormatter;

#undef CREATE_BODY_PART_FORMATTER
} // anon namespace

// FIXME: port some more BodyPartFormatters to Interface::BodyPartFormatters
void BodyPartFormatterFactoryPrivate::messageviewer_create_builtin_bodypart_formatters(BodyPartFormatterFactoryPrivate::TypeRegistry *reg)
{
    if (!reg) {
        return;
    }
    (*reg)["application"]["octet-stream"] = AnyTypeBodyPartFormatter::create();
    (*reg)["application"]["pgp"] = ApplicationPgpBodyPartFormatter::create();
    (*reg)["application"]["pkcs7-mime"] = ApplicationPkcs7MimeBodyPartFormatter::create();
    (*reg)["application"]["x-pkcs7-mime"] = ApplicationPkcs7MimeBodyPartFormatter::create();
    (*reg)["application"]["*"] = AnyTypeBodyPartFormatter::create();

    (*reg)["text"]["plain"] = TextPlainBodyPartFormatter::create();
    (*reg)["text"]["html"] = TextHtmlBodyPartFormatter::create();
    (*reg)["text"]["rtf"] = AnyTypeBodyPartFormatter::create();
    (*reg)["text"]["vcard"] = AnyTypeBodyPartFormatter::create();
    (*reg)["text"]["x-vcard"] = AnyTypeBodyPartFormatter::create();
    (*reg)["text"]["*"] = TextPlainBodyPartFormatter::create();

    (*reg)["image"]["*"] = ImageTypeBodyPartFormatter::create();

    (*reg)["message"]["rfc822"] = MessageRfc822BodyPartFormatter::create();
    (*reg)["message"]["*"] = AnyTypeBodyPartFormatter::create();

    (*reg)["multipart"]["alternative"] = MultiPartAlternativeBodyPartFormatter::create();
    (*reg)["multipart"]["encrypted"] = MultiPartEncryptedBodyPartFormatter::create();
    (*reg)["multipart"]["signed"] = MultiPartSignedBodyPartFormatter::create();
    (*reg)["multipart"]["*"] = MultiPartMixedBodyPartFormatter::create();
    (*reg)["*"]["*"] = AnyTypeBodyPartFormatter::create();
}
