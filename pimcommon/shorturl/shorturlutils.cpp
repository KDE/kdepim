/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "shorturlutils.h"
#include "pimcommon/shorturl/abstractshorturl.h"
#include "pimcommon/shorturl/googleshorturl.h"
#include "pimcommon/shorturl/tinyurlshorturl.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <KGlobal>
#include <KLocale>

#include <QDebug>

PimCommon::AbstractShortUrl *PimCommon::ShortUrlUtils::loadEngine(QObject *parent)
{
    KConfigGroup grp( KGlobal::config(), "ShortUrl" );

    PimCommon::AbstractShortUrl *engine = 0;
    PimCommon::ShortUrlUtils::EngineType type = static_cast<PimCommon::ShortUrlUtils::EngineType>(grp.readEntry("Engine", 0));
    switch (type) {
    case Google:
        engine = new PimCommon::GoogleShortUrl(parent);
        break;
    case Tinyurl:
        engine = new PimCommon::TinyurlShortUrl(parent);
        break;
    case EndListEngine:
    default:
        qDebug()<<" Engine type undefined "<<type;
        engine = new PimCommon::GoogleShortUrl(parent);
        break;
    }
    return engine;
}

QString PimCommon::ShortUrlUtils::stringFromEngineType(EngineType type)
{
    QString name;
    switch (type) {
    case Google:
        name = i18n("Google");
        break;
    case Tinyurl:
        name = i18n("Tinyurl");
        break;
    case EndListEngine:
    default:
        qDebug()<<" not supported engine type "<<type;
    }
    return name;
}
