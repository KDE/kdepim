#include "filteractionsettransporttest.h"
#include "../filteractions/filteractionsettransport.h"
#include <qtest.h>
#include <MailTransport/MailTransport/TransportComboBox>

FilterActionSetTransportTest::FilterActionSetTransportTest(QObject *parent)
    : QObject(parent)
{

}

FilterActionSetTransportTest::~FilterActionSetTransportTest()
{

}

void FilterActionSetTransportTest::shouldBeEmpty()
{
    MailCommon::FilterActionSetTransport filter;
    QVERIFY(filter.isEmpty());
}

void FilterActionSetTransportTest::shouldHaveDefaultValue()
{
    MailCommon::FilterActionSetTransport filter;
    QWidget *w = filter.createParamWidget(0);
    QCOMPARE(w->objectName(), QStringLiteral("transportcombobox"));

    MailTransport::TransportComboBox *transportCombobox = dynamic_cast<MailTransport::TransportComboBox *>(w);
    QVERIFY(transportCombobox);
}

void FilterActionSetTransportTest::shouldHaveRequiredPart()
{
    MailCommon::FilterActionSetTransport filter;
    QCOMPARE(filter.requiredPart(), MailCommon::SearchRule::CompleteMessage);
}

void FilterActionSetTransportTest::shouldHaveSieveRequires()
{
    MailCommon::FilterActionSetTransport filter;
    QCOMPARE(filter.sieveRequires(), QStringList());
}

QTEST_MAIN(FilterActionSetTransportTest)
