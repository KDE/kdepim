/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "messageheaderfilter.h"
#include <kmime/kmime_headers.h>
#include <messagecore/utils/stringutil.h>
#include <grantlee/util.h>
#include <QDebug>

QVariant MessageHeaderEmailShowLink::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape);
    Q_UNUSED(argument);
    const KMime::Types::Mailbox::List mailboxes = MessageCore::StringUtil::mailboxListFromUnicodeString(Grantlee::getSafeString(input));
    return MessageCore::StringUtil::emailAddrAsAnchor(mailboxes, MessageCore::StringUtil::DisplayFullAddress, QString(), MessageCore::StringUtil::ShowLink);
}

bool MessageHeaderEmailShowLink::isSafe() const
{
    return true;
}

QVariant MessageHeaderEmailNameOnly::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape);
    Q_UNUSED(argument);
    const KMime::Types::Mailbox::List mailboxes = MessageCore::StringUtil::mailboxListFromUnicodeString(Grantlee::getSafeString(input));
    return MessageCore::StringUtil::emailAddrAsAnchor(mailboxes, MessageCore::StringUtil::DisplayNameOnly);
}

bool MessageHeaderEmailNameOnly::isSafe() const
{
    return true;
}

QVariant MessageHeaderEmailExpandable::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    Q_UNUSED(autoescape);
    Q_UNUSED(argument);
    if (input.value<KMime::Headers::Cc *>()) {
        const QVariant val = MessageCore::StringUtil::emailAddrAsAnchor(input.value<KMime::Headers::Cc *>(), MessageCore::StringUtil::DisplayFullAddress, QString(), MessageCore::StringUtil::ShowLink,
                             MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList")
                             /*GlobalSettings::self()->numberOfAddressesToShow()*/);
        return val;
    } else if (input.value<KMime::Headers::To *>()) {
        const QVariant val = MessageCore::StringUtil::emailAddrAsAnchor(input.value<KMime::Headers::To *>(), MessageCore::StringUtil::DisplayFullAddress, QString(), MessageCore::StringUtil::ShowLink,
                             MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList")
                             /*GlobalSettings::self()->numberOfAddressesToShow()*/);
        return val;
    } else if (input.value<KMime::Headers::Bcc *>()) {
        const QVariant val = MessageCore::StringUtil::emailAddrAsAnchor(input.value<KMime::Headers::Bcc *>(), MessageCore::StringUtil::DisplayFullAddress, QString(), MessageCore::StringUtil::ShowLink,
                             MessageCore::StringUtil::ExpandableAddresses, QStringLiteral("FullToAddressList")
                             /*GlobalSettings::self()->numberOfAddressesToShow()*/);
        return val;
    } else {
        qDebug() << " invalid pointer";
    }
    return QVariant();
}

bool MessageHeaderEmailExpandable::isSafe() const
{
    return true;
}
