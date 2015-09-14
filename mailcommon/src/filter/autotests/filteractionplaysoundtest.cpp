/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "filteractionplaysoundtest.h"
#include "../filteractions/filteractionplaysound.h"
#include <qtest.h>
#include "filter/soundtestwidget.h"

FilterActionPlaySoundTest::FilterActionPlaySoundTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionPlaySoundTest::~FilterActionPlaySoundTest()
{

}

void FilterActionPlaySoundTest::shouldBeValid()
{
    MailCommon::FilterActionPlaySound filter;
    QVERIFY(filter.isEmpty());
    filter.argsFromString(QStringLiteral("foo"));
    QVERIFY(!filter.isEmpty());
}

void FilterActionPlaySoundTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionPlaySound filter;
    QWidget *w = filter.createParamWidget(0);
    QVERIFY(w);
    QCOMPARE(w->objectName(), QStringLiteral("soundwidget"));
    MailCommon::SoundTestWidget *soundTest = dynamic_cast<MailCommon::SoundTestWidget *>(w);
    QVERIFY(soundTest);
    QVERIFY(soundTest->url().isEmpty());
}

void FilterActionPlaySoundTest::shouldHaveRequiredPart()
{
    MailCommon::FilterActionPlaySound filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::Envelope);
}

void FilterActionPlaySoundTest::shouldSieveRequres()
{
    MailCommon::FilterActionPlaySound w;
    QCOMPARE(w.sieveRequires(), QStringList());

}

QTEST_MAIN(FilterActionPlaySoundTest)
