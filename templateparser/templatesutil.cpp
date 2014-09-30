/*
  Copyright (c) 2011-2014 Montel Laurent <montel@kde.org>

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

#include "templatesutil.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QStringList>
using namespace TemplateParser;

void TemplateParser::Util::deleteTemplate(const QString &id)
{
    KSharedConfig::Ptr config =
        KSharedConfig::openConfig(QLatin1String("templatesconfigurationrc"), KConfig::NoGlobals);

    const QString key = QString::fromLatin1("Templates #%1").arg(id);
    if (config->hasGroup(key)) {
        KConfigGroup group = config->group(key);
        group.deleteGroup();
        group.sync();
    }
}

QStringList TemplateParser::Util::keywordsWithArgs()
{
    QStringList keywordsWithArgs;
    keywordsWithArgs
            << QLatin1String("%REM=\".*\"%-")
            << QLatin1String("%INSERT=\".*\"")
            << QLatin1String("%SYSTEM=\".*\"")
            << QLatin1String("%PUT=\".*\"")
            << QLatin1String("%QUOTEPIPE=\".*\"")
            << QLatin1String("%MSGPIPE=\".*\"")
            << QLatin1String("%BODYPIPE=\".*\"")
            << QLatin1String("%CLEARPIPE=\".*\"")
            << QLatin1String("%TEXTPIPE=\".*\"")
            << QLatin1String("%OHEADER=\".*\"")
            << QLatin1String("%HEADER=\".*\"");

    return keywordsWithArgs;
}

QStringList TemplateParser::Util::keywords()
{
    QStringList keywords;
    keywords
            << QLatin1String("%QUOTE")
            << QLatin1String("%FORCEDPLAIN")
            << QLatin1String("%FORCEDHTML")
            << QLatin1String("%QHEADERS")
            << QLatin1String("%HEADERS")
            << QLatin1String("%TEXT")
            << QLatin1String("%OTEXTSIZE")
            << QLatin1String("%OTEXT")
            << QLatin1String("%OADDRESSEESADDR")
            << QLatin1String("%CCADDR")
            << QLatin1String("%CCNAME")
            << QLatin1String("%CCFNAME")
            << QLatin1String("%CCLNAME")
            << QLatin1String("%TOADDR")
            << QLatin1String("%TONAME")
            << QLatin1String("%TOFNAME")
            << QLatin1String("%TOLNAME")
            << QLatin1String("%TOLIST")
            << QLatin1String("%FROMADDR")
            << QLatin1String("%FROMNAME")
            << QLatin1String("%FROMFNAME")
            << QLatin1String("%FROMLNAME")
            << QLatin1String("%FULLSUBJECT")
            << QLatin1String("%FULLSUBJ")
            << QLatin1String("%MSGID")
            << QLatin1String("%HEADER( ")
            << QLatin1String("%OCCADDR")
            << QLatin1String("%OCCNAME")
            << QLatin1String("%OCCFNAME")
            << QLatin1String("%OCCLNAME")
            << QLatin1String("%OTOADDR")
            << QLatin1String("%OTONAME")
            << QLatin1String("%OTOFNAME")
            << QLatin1String("%OTOLNAME")
            << QLatin1String("%OTOLIST")
            << QLatin1String("%OTO")
            << QLatin1String("%OFROMADDR")
            << QLatin1String("%OFROMNAME")
            << QLatin1String("%OFROMFNAME")
            << QLatin1String("%OFROMLNAME")
            << QLatin1String("%OFULLSUBJECT")
            << QLatin1String("%OFULLSUBJ")
            << QLatin1String("%OMSGID")
            << QLatin1String("%DATEEN")
            << QLatin1String("%DATESHORT")
            << QLatin1String("%DATE")
            << QLatin1String("%DOW")
            << QLatin1String("%TIMELONGEN")
            << QLatin1String("%TIMELONG")
            << QLatin1String("%TIME")
            << QLatin1String("%ODATEEN")
            << QLatin1String("%ODATESHORT")
            << QLatin1String("%ODATE")
            << QLatin1String("%ODOW")
            << QLatin1String("%OTIMELONGEN")
            << QLatin1String("%OTIMELONG")
            << QLatin1String("%OTIME")
            << QLatin1String("%BLANK")
            << QLatin1String("%NOP")
            << QLatin1String("%CLEAR")
            << QLatin1String("%DEBUGOFF")
            << QLatin1String("%DEBUG")
            << QLatin1String("%CURSOR")
            << QLatin1String("%SIGNATURE");
    return keywords;
}
