/*
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "messagehelper.h"
#include "kdepim-version.h"
#include "utils/util.h"
#include "settings/messagecomposersettings.h"

#include "MessageCore/MailingList"
#include "MessageCore/StringUtil"

#include <KProtocolManager>
#include <KMime/Message>
#include <kmime/kmime_mdn.h>
#include <kmime/kmime_dateformatter.h>
#include <kmime/kmime_headers.h>
#include <kidentitymanagement/identitymanager.h>
#include <kidentitymanagement/identity.h>
#include "messagecomposer_debug.h"

using namespace MessageCore;

namespace MessageHelper
{

void initHeader(const KMime::Message::Ptr &message, const KIdentityManagement::IdentityManager *identMan, uint id)
{
    applyIdentity(message, identMan, id);
    message->removeHeader<KMime::Headers::To>();
    message->removeHeader<KMime::Headers::Subject>();
    message->date()->setDateTime(QDateTime::currentDateTime());

    const QStringList extraInfo = QStringList() << QString::fromLocal8Bit(KDEPIM_GIT_REVISION_STRING) << QString::fromLocal8Bit(KDEPIM_GIT_LAST_CHANGE);

    message->userAgent()->fromUnicodeString(KProtocolManager::userAgentForApplication(QString::fromLocal8Bit("KMail"), QString::fromLocal8Bit(KDEPIM_GIT_REVISION_STRING), extraInfo), QLatin1String("utf-8").latin1());
    // This will allow to change Content-Type:
    message->contentType()->setMimeType("text/plain");
}

void initFromMessage(const KMime::Message::Ptr &msg, const KMime::Message::Ptr &origMsg,
                     KIdentityManagement::IdentityManager *identMan, uint id, bool idHeaders)
{
    if (idHeaders) {
        MessageHelper::initHeader(msg, identMan, id);
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Identity");
        header->fromUnicodeString(QString::number(id), "utf-8");
        msg->setHeader(header);
    }

    if (auto hdr = origMsg->headerByType("X-KMail-Transport")) {
        const QString transport = hdr->asUnicodeString();
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Transport");
        header->fromUnicodeString(transport, "utf-8");
        msg->setHeader(header);
    }
}

void applyIdentity(const KMime::Message::Ptr &message, const KIdentityManagement::IdentityManager *identMan, uint id)
{
    const KIdentityManagement::Identity &ident =
        identMan->identityForUoidOrDefault(id);

    if (ident.fullEmailAddr().isEmpty()) {
        message->removeHeader<KMime::Headers::From>();
    } else {
        message->from()->addAddress(ident.primaryEmailAddress().toUtf8(), ident.fullName());
    }

    if (ident.replyToAddr().isEmpty()) {
        message->removeHeader<KMime::Headers::ReplyTo>();
    } else {
        message->replyTo()->addAddress(ident.replyToAddr().toUtf8());
    }

    if (ident.bcc().isEmpty()) {
        message->removeHeader<KMime::Headers::Bcc>();
    } else {
        const auto mailboxes = KMime::Types::Mailbox::listFromUnicodeString(ident.bcc());
        foreach (const KMime::Types::Mailbox &mailbox, mailboxes) {
            message->bcc()->addAddress(mailbox);
        }
    }

    if (ident.cc().isEmpty()) {
        message->removeHeader<KMime::Headers::Cc>();
    } else {
        const auto mailboxes = KMime::Types::Mailbox::listFromUnicodeString(ident.cc());
        foreach (const KMime::Types::Mailbox &mailbox, mailboxes) {
            message->cc()->addAddress(mailbox);
        }
    }

    if (ident.organization().isEmpty()) {
        message->removeHeader<KMime::Headers::Organization>();
    } else {
        KMime::Headers::Organization *const organization = new KMime::Headers::Organization;
        organization->fromUnicodeString(ident.organization(), "utf-8");
        message->setHeader(organization);
    }

    if (ident.isDefault()) {
        message->removeHeader("X-KMail-Identity");
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Identity");
        header->fromUnicodeString(QString::number(ident.uoid()), "utf-8");
        message->setHeader(header);
    }

    if (ident.transport().isEmpty()) {
        message->removeHeader("X-KMail-Transport");
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Transport");
        header->fromUnicodeString(ident.transport(), "utf-8");
        message->setHeader(header);
    }

    if (ident.fcc().isEmpty()) {
        message->removeHeader("X-KMail-Fcc");
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Fcc");
        header->fromUnicodeString(ident.fcc(), "utf-8");
        message->setHeader(header);
    }

    if (ident.drafts().isEmpty()) {
        message->removeHeader("X-KMail-Drafts");
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Drafts");
        header->fromUnicodeString(ident.drafts(), "utf-8");
        message->setHeader(header);
    }

    if (ident.templates().isEmpty()) {
        message->removeHeader("X-KMail-Templates");
    } else {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-Templates");
        header->fromUnicodeString(ident.templates(), "utf-8");
        message->setHeader(header);
    }

    if (ident.disabledFcc()) {
        KMime::Headers::Generic *header = new KMime::Headers::Generic("X-KMail-FccDisabled");
        header->fromUnicodeString(QStringLiteral("true"), "utf-8");
        message->setHeader(header);
    } else {
        message->removeHeader("X-KMail-FccDisabled");
    }
}

KMime::Types::AddrSpecList extractAddrSpecs(const KMime::Message::Ptr &msg, const QByteArray &header)
{
    KMime::Types::AddrSpecList result;
    if (!msg->headerByType(header)) {
        return result;
    }

    KMime::Types::AddressList al =
        MessageCore::StringUtil::splitAddressField(msg->headerByType(header)->asUnicodeString().toUtf8());
    KMime::Types::AddressList::const_iterator alend(al.constEnd());
    for (KMime::Types::AddressList::const_iterator ait = al.constBegin(); ait != alend; ++ait) {
        KMime::Types::MailboxList::const_iterator mitEnd((*ait).mailboxList.constEnd());
        for (KMime::Types::MailboxList::const_iterator mit = (*ait).mailboxList.constBegin(); mit != mitEnd; ++mit) {
            result.push_back((*mit).addrSpec());
        }
    }
    return result;
}

QString cleanSubject(const KMime::Message::Ptr &msg)
{
    return cleanSubject(msg, MessageComposer::MessageComposerSettings::self()->replyPrefixes() + MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                        true, QString()).trimmed();
}

QString cleanSubject(const KMime::Message::Ptr &msg, const QStringList &prefixRegExps,
                     bool replace, const QString &newPrefix)
{
    return replacePrefixes(msg->subject()->asUnicodeString(), prefixRegExps, replace,
                           newPrefix);
}

QString forwardSubject(const KMime::Message::Ptr &msg)
{
    return cleanSubject(msg, MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                        MessageComposer::MessageComposerSettings::self()->replaceForwardPrefix(), QStringLiteral("Fwd:"));
}

QString replySubject(const KMime::Message::Ptr &msg)
{
    return cleanSubject(msg, MessageComposer::MessageComposerSettings::self()->replyPrefixes(),
                        MessageComposer::MessageComposerSettings::self()->replaceReplyPrefix(), QStringLiteral("Re:"));
}

QString replacePrefixes(const QString &str, const QStringList &prefixRegExps,
                        bool replace, const QString &newPrefix)
{
    bool recognized = false;
    // construct a big regexp that
    // 1. is anchored to the beginning of str (sans whitespace)
    // 2. matches at least one of the part regexps in prefixRegExps
    QString bigRegExp = QStringLiteral("^(?:\\s+|(?:%1))+\\s*")
                        .arg(prefixRegExps.join(QStringLiteral(")|(?:")));
    QRegExp rx(bigRegExp, Qt::CaseInsensitive);
    if (rx.isValid()) {
        QString tmp = str;
        if (rx.indexIn(tmp) == 0) {
            recognized = true;
            if (replace) {
                return tmp.replace(0, rx.matchedLength(), newPrefix + QLatin1String(" "));
            }
        }
    } else {
        qCWarning(MESSAGECOMPOSER_LOG) << "bigRegExp = \""
                                       << bigRegExp << "\"\n"
                                       << "prefix regexp is invalid!";
        // try good ole Re/Fwd:
        recognized = str.startsWith(newPrefix);
    }

    if (!recognized) {
        return newPrefix + QLatin1String(" ") + str;
    } else {
        return str;
    }
}

void setAutomaticFields(const KMime::Message::Ptr &msg, bool aIsMulti)
{
    auto header = msg->header<KMime::Headers::MIMEVersion>(true);
    header->from7BitString("1.0");

    if (aIsMulti || msg->contents().size() > 1) {
        // Set the type to 'Multipart' and the subtype to 'Mixed'
        msg->contentType()->setMimeType("multipart/mixed");
        // Create a random printable string and set it as the boundary parameter
        msg->contentType()->setBoundary(KMime::multiPartBoundary());
    }
}

QString stripOffPrefixes(const QString &str)
{
    return replacePrefixes(str, MessageComposer::MessageComposerSettings::self()->replyPrefixes() + MessageComposer::MessageComposerSettings::self()->forwardPrefixes(),
                           true, QString()).trimmed();
}

}
