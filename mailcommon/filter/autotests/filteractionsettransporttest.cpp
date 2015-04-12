#include "filteractionsettransporttest.h"
#include "../filteractions/filteractionsettransport.h"
#include <qtest_kde.h>
#include <KDE/Mailtransport/TransportComboBox>

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
    QCOMPARE(w->objectName(), QLatin1String("transportcombobox"));

    MailTransport::TransportComboBox *transportCombobox = dynamic_cast<MailTransport::TransportComboBox *>(w);
    QVERIFY(transportCombobox);
}


QTEST_KDEMAIN(FilterActionSetTransportTest, GUI)
