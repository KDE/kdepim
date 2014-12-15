/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include "pimcommon/shorturl/shorturlutils.h"
#include "pimcommon/shorturl/abstractshorturl.h"
#include <qtest_kde.h>
#include <QSignalSpy>

ShortUrlEnginesTest::ShortUrlEnginesTest(QObject *parent)
    : QObject(parent)
{

}

ShortUrlEnginesTest::~ShortUrlEnginesTest()
{

}

void ShortUrlEnginesTest::shouldCreateEngine()
{
    for (int i=0; i <PimCommon::ShortUrlUtils::EndListEngine; ++i) {
        PimCommon::AbstractShortUrl *abstrShortUrl = PimCommon::ShortUrlUtils::loadEngine(static_cast<PimCommon::ShortUrlUtils::EngineType>(i), 0);
        QVERIFY(abstrShortUrl);
        delete abstrShortUrl;
    }
}

void ShortUrlEnginesTest::shouldtestenginesData()
{

}

void ShortUrlEnginesTest::shouldTestEngines()
{

}

QTEST_KDEMAIN(ShortUrlEnginesTest, NoGUI)
