/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "shorturlenginestest.h"
#include "shorturl/shorturlutils.h"
#include "shorturl/abstractshorturl.h"
#include <qtest.h>
#include <QSignalSpy>

Q_DECLARE_METATYPE(PimCommon::ShortUrlUtils::EngineType)
ShortUrlEnginesTest::ShortUrlEnginesTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<PimCommon::ShortUrlUtils::EngineType>();

}

ShortUrlEnginesTest::~ShortUrlEnginesTest()
{

}

void ShortUrlEnginesTest::shouldCreateEngine()
{
    for (int i = 0; i < PimCommon::ShortUrlUtils::EndListEngine; ++i) {
        PimCommon::AbstractShortUrl *abstrShortUrl = PimCommon::ShortUrlUtils::loadEngine(static_cast<PimCommon::ShortUrlUtils::EngineType>(i), Q_NULLPTR);
        QVERIFY(abstrShortUrl);
        delete abstrShortUrl;
    }
}

void ShortUrlEnginesTest::shouldTestEngines_data()
{
    QTest::addColumn<PimCommon::ShortUrlUtils::EngineType>("engine");
    QTest::addColumn<QString>("longurl");
    QTest::addColumn<QString>("shorturl");

    //QTest::newRow("google url") << PimCommon::ShortUrlUtils::Google << QStringLiteral("http://www.kde.org") << QStringLiteral("http://goo.gl/sMKw");
    QTest::newRow("tiny url") << PimCommon::ShortUrlUtils::Tinyurl << QStringLiteral("http://www.kde.org") << QStringLiteral("http://tinyurl.com/l6l0");
    //WE can't test migreme...
    //QTest::newRow("migreme url") << PimCommon::ShortUrlUtils::MigreMe << QStringLiteral("http://www.kde.org") << QStringLiteral("http://migre.me/nwh5a");
    //QTest::newRow("triopAB url") << PimCommon::ShortUrlUtils::TriopAB << QStringLiteral("http://www.kde.org") << QStringLiteral("http://to.ly/51UP");
    //FIX ME QTest::newRow("ur1 url") << PimCommon::ShortUrlUtils::Ur1Ca << QStringLiteral("http://www.kde.org") << QStringLiteral("http://to.ly/51UP");
}

void ShortUrlEnginesTest::shouldTestEngines()
{
    QFETCH(PimCommon::ShortUrlUtils::EngineType, engine);
    QFETCH(QString, longurl);
    QFETCH(QString, shorturl);
    PimCommon::AbstractShortUrl *abstrShortUrl = PimCommon::ShortUrlUtils::loadEngine(engine, Q_NULLPTR);
    QSignalSpy spy(abstrShortUrl, SIGNAL(shortUrlDone(QString)));
    abstrShortUrl->shortUrl(longurl);
    abstrShortUrl->start();
    QVERIFY(spy.wait(10000));
    QCOMPARE(spy.at(0).count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), shorturl);
}

QTEST_MAIN(ShortUrlEnginesTest)
