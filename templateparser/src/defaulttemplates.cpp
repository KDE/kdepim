/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "defaulttemplates.h"

#include <KLocalizedString>

using namespace TemplateParser;

QString DefaultTemplates::defaultNewMessage()
{
    return
        QStringLiteral("%REM=\"") + i18n("Default new message template") + QStringLiteral("\"%-\n") + QStringLiteral("%BLANK");
}

QString DefaultTemplates::defaultReply()
{
    return
        QStringLiteral("%REM=\"") + i18n("Default reply template") + QStringLiteral("\"%-\n") +
        i18nc("Default reply template."
              "%1: date of original message, %2: time of original message, "
              "%3: quoted text of original message, %4: cursor Position",
              "On %1 %2 you wrote:\n"
              "%3\n"
              "%4", QStringLiteral("%ODATE"), QStringLiteral("%OTIMELONG"), QStringLiteral("%QUOTE"), QStringLiteral("%CURSOR"));
}

QString DefaultTemplates::defaultReplyAll()
{
    return
        QStringLiteral("%REM=\"") + i18n("Default reply all template") + QStringLiteral("\"%-\n") +
        i18nc("Default reply all template: %1: date, %2: time, %3: name of original sender, "
              "%4: quoted text of original message, %5: cursor position",
              "On %1 %2 %3 wrote:\n"
              "%4\n"
              "%5",
              QStringLiteral("%ODATE"), QStringLiteral("%OTIMELONG"), QStringLiteral("%OFROMNAME"), QStringLiteral("%QUOTE"), QStringLiteral("%CURSOR"));
}

QString DefaultTemplates::defaultForward()
{
    return
        QStringLiteral("%REM=\"") + i18n("Default forward template") + QStringLiteral("\"%-\n") +
        i18nc("Default forward template: %1: subject of original message, "
              "%2: date of original message, "
              "%3: time of original message, "
              "%4: mail address of original sender, "
              "%5: original message text",
              "\n"
              "----------  Forwarded Message  ----------\n"
              "\n"
              "Subject: %1\n"
              "Date: %2, %3\n"
              "From: %4\n"
              "%OADDRESSEESADDR\n"
              "\n"
              "%5\n"
              "-----------------------------------------",
              QStringLiteral("%OFULLSUBJECT"), QStringLiteral("%ODATE"), QStringLiteral("%OTIMELONG"), QStringLiteral("%OFROMADDR"), QStringLiteral("%TEXT"));
}

QString DefaultTemplates::defaultQuoteString()
{
    return QStringLiteral("> ");
}
