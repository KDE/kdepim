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
    QCOMPARE(job.size(), 80);
    QCOMPARE(job.hasGravatar(), false);
    QCOMPARE(job.pixmap().isNull(), true);
    QCOMPARE(job.useDefaultPixmap(), false);
    QCOMPARE(job.useCache(), false);
}

void GravatarResolvUrlJobTest::shouldChangeValue()
{
    PimCommon::GravatarResolvUrlJob job;
    bool useCache = true;
    job.setUseCache(useCache);
    QCOMPARE(job.useCache(), useCache);
    useCache = false;
    job.setUseCache(useCache);
    QCOMPARE(job.useCache(), useCache);

    bool useDefaultPixmap = true;
    job.setUseDefaultPixmap(useDefaultPixmap);
    QCOMPARE(job.useDefaultPixmap(), useDefaultPixmap );

    useDefaultPixmap = false;
    job.setUseDefaultPixmap(useDefaultPixmap);
    QCOMPARE(job.useDefaultPixmap(), useDefaultPixmap );
}

void GravatarResolvUrlJobTest::shouldChangeSize()
{
    PimCommon::GravatarResolvUrlJob job;
    int size = 50;
    job.setSize(size);
    QCOMPARE(job.size(), size);
    size = 0;
    job.setSize(size);
    QCOMPARE(job.size(), 80);

    size = 10;
    job.setSize(size);
    QCOMPARE(job.size(), size);

    size = 2048;
    job.setSize(size);
    QCOMPARE(job.size(), size);

    size = 4096;
    job.setSize(size);
    QCOMPARE(job.size(), 2048);
}

void GravatarResolvUrlJobTest::shouldAddSizeInUrl()
{
    PimCommon::GravatarResolvUrlJob job;
    job.setEmail(QLatin1String("foo@kde.org"));
    job.setSize(1024);
    KUrl url = job.generateGravatarUrl();
    QCOMPARE(url, KUrl("http://www.gravatar.com:80/avatar/89b4e14cf2fc6d426275c019c6dc9de6?d=404&s=1024"));
}

void GravatarResolvUrlJobTest::shouldUseDefaultPixmap()
{
    PimCommon::GravatarResolvUrlJob job;
    job.setEmail(QLatin1String("foo@kde.org"));
    job.setSize(1024);
    job.setUseDefaultPixmap(true);
    KUrl url = job.generateGravatarUrl();
    QCOMPARE(url, KUrl("http://www.gravatar.com:80/avatar/89b4e14cf2fc6d426275c019c6dc9de6?s=1024"));
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
    QTest::addColumn<QString>("calculedhash");
    QTest::addColumn<KUrl>("output");
    QTest::newRow("empty") << QString() << QString() << KUrl();
    QTest::newRow("no domain") << QString(QLatin1String("foo")) << QString() << KUrl();
    QTest::newRow("validemail") << QString(QLatin1String("foo@kde.org")) << QString(QLatin1String("89b4e14cf2fc6d426275c019c6dc9de6")) << KUrl("http://www.gravatar.com:80/avatar/89b4e14cf2fc6d426275c019c6dc9de6?d=404");
}

void GravatarResolvUrlJobTest::shouldGenerateGravatarUrl()
{
    QFETCH( QString, input );
    QFETCH( QString, calculedhash);
    QFETCH( KUrl, output );
    PimCommon::GravatarResolvUrlJob job;
    job.setEmail(input);
    KUrl url = job.generateGravatarUrl();
    QCOMPARE(calculedhash, job.calculatedHash());
    QCOMPARE(url, output);
}


QTEST_KDEMAIN(GravatarResolvUrlJobTest, GUI)