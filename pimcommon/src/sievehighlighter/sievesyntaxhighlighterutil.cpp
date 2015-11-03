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

#include "sievesyntaxhighlighterutil.h"

namespace PimCommon
{
QStringList SieveSyntaxHighlighterUtil::fullCapabilities() const
{
    return  QStringList()
            << QStringLiteral("mboxmetadata")
            << QStringLiteral("body")
            << QStringLiteral("extlists")
            << QStringLiteral("envelope")
            << QStringLiteral("redirect")
            << QStringLiteral("fileinto")
            << QStringLiteral("editheader")
            << QStringLiteral("reject")
            << QStringLiteral("ereject")
            << QStringLiteral("imapflags")
            << QStringLiteral("imap4flags")
            << QStringLiteral("enotify")
            << QStringLiteral("date")
            << QStringLiteral("copy")
            << QStringLiteral("mailbox")
            << QStringLiteral("spamtest")
            << QStringLiteral("spamtestplus")
            << QStringLiteral("virustest")
            << QStringLiteral("vacation")
            << QStringLiteral("vacation-seconds")
            << QStringLiteral("ihave")
            << QStringLiteral("subaddress")
            << QStringLiteral("environment")
            << QStringLiteral("enclose")
            << QStringLiteral("replace")
            << QStringLiteral("include")
            << QStringLiteral("extracttext")
            << QStringLiteral("metadata")
            << QStringLiteral("convert")
            << QStringLiteral("foreverypart")
            << QStringLiteral("variables")
            << QStringLiteral("servermetadata")
            << QStringLiteral("regex")
            << QStringLiteral("relational")
            << QStringLiteral("comparator-i;ascii-numeric")
            << QStringLiteral("comparator-i;unicode-casemap")
            << QStringLiteral("encoded-character");
}
}
