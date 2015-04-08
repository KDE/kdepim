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

#include "filteractionwithurltest.h"
#include "../filteractions/filteractionwithurl.h"
#include <KUrlRequester>
#include <KLineEdit>
#include <qtest.h>

class TestFilterActionWithUrl : public MailCommon::FilterActionWithUrl
{
public:
    TestFilterActionWithUrl()
        : MailCommon::FilterActionWithUrl(QStringLiteral("test"), QStringLiteral("label"))
    {

    }
    FilterAction::ReturnCode process(MailCommon::ItemContext &context , bool) const
    {
        return GoOn;
    }

    MailCommon::SearchRule::RequiredPart requiredPart() const
    {
        return MailCommon::SearchRule::CompleteMessage;
    }
};

FilterActionWithUrlTest::FilterActionWithUrlTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionWithUrlTest::~FilterActionWithUrlTest()
{

}

void FilterActionWithUrlTest::shouldHaveDefaultValue()
{
    TestFilterActionWithUrl filter;
    QWidget *w = filter.createParamWidget(0);
    KUrlRequester *requester = w->findChild<KUrlRequester *>(QStringLiteral("requester"));
    QVERIFY(requester);
    QToolButton *toolButton = w->findChild<QToolButton *>(QStringLiteral("helpbutton"));
    QVERIFY(toolButton);
}

void FilterActionWithUrlTest::shouldClearWidget()
{
    TestFilterActionWithUrl filter;
    QWidget *w = filter.createParamWidget(0);
    KUrlRequester *requester = w->findChild<KUrlRequester *>(QStringLiteral("requester"));
    requester->setUrl(QUrl::fromLocalFile("/foo/bla"));
    QVERIFY(!requester->url().isEmpty());
    filter.clearParamWidget(w);
    QVERIFY(requester->url().isEmpty());
}

void FilterActionWithUrlTest::shouldAddValue()
{
    TestFilterActionWithUrl filter;
    QWidget *w = filter.createParamWidget(0);
    KUrlRequester *requester = w->findChild<KUrlRequester *>(QStringLiteral("requester"));
    filter.argsFromString(QStringLiteral("/foo"));
    filter.setParamWidgetValue(w);
    QCOMPARE(requester->lineEdit()->text(), QStringLiteral("/foo"));
}

void FilterActionWithUrlTest::shouldApplyValue()
{
    TestFilterActionWithUrl filter;
    QWidget *w = filter.createParamWidget(0);
    filter.argsFromString(QStringLiteral("foo"));
    filter.setParamWidgetValue(w);
    filter.applyParamWidgetValue(w);
    QCOMPARE(filter.argsAsString(), QStringLiteral("foo"));
}

void FilterActionWithUrlTest::shouldTestUrl_data()
{
    QTest::addColumn<QString>("urlstr");
    QTest::addColumn<QString>("output");
    QTest::newRow("fullpath") <<  QString(QStringLiteral("/usr/bin/ls")) << QString(QStringLiteral("/usr/bin/ls"));
    QTest::newRow("local") <<  QString(QStringLiteral("ls")) << QString(QStringLiteral("ls"));
    QTest::newRow("localwithargument") <<  QString(QStringLiteral("ls -l")) << QString(QStringLiteral("ls -l"));
    QTest::newRow("fullpathwithargument") <<  QString(QStringLiteral("/usr/bin/ls -l")) << QString(QStringLiteral("/usr/bin/ls -l"));
    QTest::newRow("url") <<  QString(QStringLiteral("file:///usr/bin/ls -l")) << QString(QStringLiteral("/usr/bin/ls -l"));
    QTest::newRow("url2") <<  QString(QStringLiteral("/usr/bin/ls -l")) << QString(QStringLiteral("/usr/bin/ls -l"));
}

void FilterActionWithUrlTest::shouldTestUrl()
{
    QFETCH(QString, urlstr);
    QFETCH(QString, output);
    TestFilterActionWithUrl filter;
    QWidget *w = filter.createParamWidget(0);
    filter.argsFromString(urlstr);
    filter.setParamWidgetValue(w);
    filter.applyParamWidgetValue(w);
    QCOMPARE(filter.argsAsString(), output);
}

QTEST_MAIN(FilterActionWithUrlTest)
