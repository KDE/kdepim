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
    lst << QLatin1String("<div>")
        << QLatin1String("class=\"fancy header\"")
        << QLatin1String("header.absoluteThemePath")
        << QLatin1String("header.subjecti18n")
        << QLatin1String("header.subject")
        << QLatin1String("header.replyToi18n")
        << QLatin1String("header.replyTo")
        << QLatin1String("header.replyToStr")
        << QLatin1String("header.toi18n")
        << QLatin1String("header.to")
        << QLatin1String("header.toStr")
        << QLatin1String("header.cci18n")
        << QLatin1String("header.cc")
        << QLatin1String("header.ccStr")
        << QLatin1String("header.bcci18n")
        << QLatin1String("header.bcc")
        << QLatin1String("header.bccStr")
        << QLatin1String("header.fromi18n")
        << QLatin1String("header.from")
        << QLatin1String("header.fromStr")
        << QLatin1String("header.spamHTML")
        << QLatin1String("header.spamstatusi18n")
        << QLatin1String("header.datei18n")
        << QLatin1String("header.dateshort")
        << QLatin1String("header.date")
        << QLatin1String("header.useragent")
        << QLatin1String("header.xmailer")
        << QLatin1String("header.resentfrom")
        << QLatin1String("header.resentfromi18n")
        << QLatin1String("header.organization")
        << QLatin1String("header.vcardname")
        << QLatin1String("header.activecolordark")
        << QLatin1String("header.fontcolor")
        << QLatin1String("header.linkcolor")
        << QLatin1String("header.photowidth")
        << QLatin1String("header.photoheight")
        << QLatin1String("header.applicationDir")
        << QLatin1String("header.subjectDir")
        << QLatin1String("header.photourl")
        << QLatin1String("header.isprinting")
        << QLatin1String("header.vcardi18n")
        << QLatin1String("header.resentto")
        << QLatin1String("header.resenttoi18n");
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst << QLatin1String("showlink")
        << QLatin1String("nameonly")
        << QLatin1String("Safe");
    return lst;
}
