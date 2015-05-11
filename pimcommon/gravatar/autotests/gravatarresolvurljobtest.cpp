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

#include "gravatarresolvurljobtest.h"
#include "gravatar/gravatarresolvurljob.h"
#include <qtest_kde.h>

GravatarResolvUrlJobTest::GravatarResolvUrlJobTest(QObject *parent)
    : QObject(parent)
{

}

GravatarResolvUrlJobTest::~GravatarResolvUrlJobTest()
{

}

void GravatarResolvUrlJobTest::shouldHaveDefaultValue()
{
    PimCommon::GravatarResolvUrlJob job;
    QVERIFY(job.email().isEmpty());
}

void GravatarResolvUrlJobTest::shouldNotStart()
{
    PimCommon::GravatarResolvUrlJob job;
    QVERIFY(!job.canStart());

    job.setEmail(QLatin1String("foo"));
    QVERIFY(!job.canStart());

    job.setEmail(QLatin1String(" "));
    QVERIFY(!job.canStart());

    job.setEmail(QLatin1String("foo@kde.org"));
    QVERIFY(job.canStart());
}


void GravatarResolvUrlJobTest::shouldGenerateGravatarUrl_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<KUrl>("output");
    QTest::newRow("empty") << QString() << KUrl();
    QTest::newRow("no domain") << QString(QLatin1String("foo")) << KUrl();
    QTest::newRow("validemail") << QString(QLatin1String("foo@kde.org")) << KUrl("http://www.gravatar.com:80/avatar/89b4e14cf2fc6d426275c019c6dc9de6?d=404");
}

void GravatarResolvUrlJobTest::shouldGenerateGravatarUrl()
{
    QFETCH( QString, input );
    QFETCH( KUrl, output );
    PimCommon::GravatarResolvUrlJob job;
    job.setEmail(input);
    KUrl url = job.generateGravatarUrl();
    QCOMPARE(url, output);
}


QTEST_KDEMAIN(GravatarResolvUrlJobTest, NoGUI)
