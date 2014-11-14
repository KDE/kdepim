/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "defaultcompletion.h"

QStringList DefaultCompletion::defaultCompetion()
{
    //TODO add to highlighter
    QStringList lst;
    lst << QStringLiteral("<div>")
        << QStringLiteral("class=\"fancy header\"")
        << QStringLiteral("header.absoluteThemePath")
        << QStringLiteral("header.subjecti18n")
        << QStringLiteral("header.subject")
        << QStringLiteral("header.replyToi18n")
        << QStringLiteral("header.replyTo")
        << QStringLiteral("header.replyToStr")
        << QStringLiteral("header.toi18n")
        << QStringLiteral("header.to")
        << QStringLiteral("header.toStr")
        << QStringLiteral("header.cci18n")
        << QStringLiteral("header.cc")
        << QStringLiteral("header.ccStr")
        << QStringLiteral("header.bcci18n")
        << QStringLiteral("header.bcc")
        << QStringLiteral("header.bccStr")
        << QStringLiteral("header.fromi18n")
        << QStringLiteral("header.from")
        << QStringLiteral("header.fromStr")
        << QStringLiteral("header.spamHTML")
        << QStringLiteral("header.spamstatusi18n")
        << QStringLiteral("header.datei18n")
        << QStringLiteral("header.dateshort")
        << QStringLiteral("header.date")
        << QStringLiteral("header.useragent")
        << QStringLiteral("header.xmailer")
        << QStringLiteral("header.resentfrom")
        << QStringLiteral("header.resentfromi18n")
        << QStringLiteral("header.organization")
        << QStringLiteral("header.vcardname")
        << QStringLiteral("header.activecolordark")
        << QStringLiteral("header.fontcolor")
        << QStringLiteral("header.linkcolor")
        << QStringLiteral("header.photowidth")
        << QStringLiteral("header.photoheight")
        << QStringLiteral("header.applicationDir")
        << QStringLiteral("header.subjectDir")
        << QStringLiteral("header.photourl")
        << QStringLiteral("header.isprinting")
        << QStringLiteral("header.vcardi18n")
        << QStringLiteral("header.resentto")
        << QStringLiteral("header.resenttoi18n");
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst << QStringLiteral("showlink")
        << QStringLiteral("nameonly")
        << QStringLiteral("Safe");
    return lst;
}
