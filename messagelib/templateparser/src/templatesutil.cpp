/*
  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

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
        KSharedConfig::openConfig(QStringLiteral("templatesconfigurationrc"), KConfig::NoGlobals);

    const QString key = QStringLiteral("Templates #%1").arg(id);
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
            << QStringLiteral("%REM=\"\"%-")
            << QStringLiteral("%INSERT=\"\"")
            << QStringLiteral("%SYSTEM=\"\"")
            << QStringLiteral("%QUOTEPIPE=\"\"")
            << QStringLiteral("%MSGPIPE=\"\"")
            << QStringLiteral("%BODYPIPE=\"\"")
            << QStringLiteral("%CLEARPIPE=\"\"")
            << QStringLiteral("%TEXTPIPE=\"\"")
            << QStringLiteral("%OHEADER=\"\"")
            << QStringLiteral("%HEADER=\"\"")
            << QStringLiteral("%DICTIONARYLANGUAGE=\"\"")
            << QStringLiteral("%LANGUAGE=\"\"");
    return keywordsWithArgs;
}

QStringList TemplateParser::Util::keywordsWithArgsForCompleter()
{
    QStringList keywordsWithArgs;
    keywordsWithArgs
            << QStringLiteral("%REM=\".*\"%-")
            << QStringLiteral("%INSERT=\".*\"")
            << QStringLiteral("%SYSTEM=\".*\"")
            << QStringLiteral("%QUOTEPIPE=\".*\"")
            << QStringLiteral("%MSGPIPE=\".*\"")
            << QStringLiteral("%BODYPIPE=\".*\"")
            << QStringLiteral("%CLEARPIPE=\".*\"")
            << QStringLiteral("%TEXTPIPE=\".*\"")
            << QStringLiteral("%OHEADER=\".*\"")
            << QStringLiteral("%HEADER=\".*\"")
            << QStringLiteral("%DICTIONARYLANGUAGE=\".*\"")
            << QStringLiteral("%LANGUAGE=\".*\"");
    return keywordsWithArgs;
}

QStringList TemplateParser::Util::keywords()
{
    QStringList keywords;
    keywords
            << QStringLiteral("%QUOTE")
            << QStringLiteral("%FORCEDPLAIN")
            << QStringLiteral("%FORCEDHTML")
            << QStringLiteral("%QHEADERS")
            << QStringLiteral("%HEADERS")
            << QStringLiteral("%TEXT")
            << QStringLiteral("%OTEXTSIZE")
            << QStringLiteral("%OTEXT")
            << QStringLiteral("%OADDRESSEESADDR")
            << QStringLiteral("%CCADDR")
            << QStringLiteral("%CCNAME")
            << QStringLiteral("%CCFNAME")
            << QStringLiteral("%CCLNAME")
            << QStringLiteral("%TOADDR")
            << QStringLiteral("%TONAME")
            << QStringLiteral("%TOFNAME")
            << QStringLiteral("%TOLNAME")
            << QStringLiteral("%TOLIST")
            << QStringLiteral("%FROMADDR")
            << QStringLiteral("%FROMNAME")
            << QStringLiteral("%FROMFNAME")
            << QStringLiteral("%FROMLNAME")
            << QStringLiteral("%FULLSUBJECT")
            << QStringLiteral("%MSGID")
            << QStringLiteral("%HEADER\\( ")
            << QStringLiteral("%OCCADDR")
            << QStringLiteral("%OCCNAME")
            << QStringLiteral("%OCCFNAME")
            << QStringLiteral("%OCCLNAME")
            << QStringLiteral("%OTOADDR")
            << QStringLiteral("%OTONAME")
            << QStringLiteral("%OTOFNAME")
            << QStringLiteral("%OTOLNAME")
            << QStringLiteral("%OTOLIST")
            << QStringLiteral("%OTO")
            << QStringLiteral("%OFROMADDR")
            << QStringLiteral("%OFROMNAME")
            << QStringLiteral("%OFROMFNAME")
            << QStringLiteral("%OFROMLNAME")
            << QStringLiteral("%OFULLSUBJECT")
            << QStringLiteral("%OFULLSUBJ")
            << QStringLiteral("%OMSGID")
            << QStringLiteral("%DATEEN")
            << QStringLiteral("%DATESHORT")
            << QStringLiteral("%DATE")
            << QStringLiteral("%DOW")
            << QStringLiteral("%TIMELONGEN")
            << QStringLiteral("%TIMELONG")
            << QStringLiteral("%TIME")
            << QStringLiteral("%ODATEEN")
            << QStringLiteral("%ODATESHORT")
            << QStringLiteral("%ODATE")
            << QStringLiteral("%ODOW")
            << QStringLiteral("%OTIMELONGEN")
            << QStringLiteral("%OTIMELONG")
            << QStringLiteral("%OTIME")
            << QStringLiteral("%BLANK")
            << QStringLiteral("%NOP")
            << QStringLiteral("%CLEAR")
            << QStringLiteral("%DEBUGOFF")
            << QStringLiteral("%DEBUG")
            << QStringLiteral("%CURSOR")
            << QStringLiteral("%SIGNATURE");
    return keywords;
}
