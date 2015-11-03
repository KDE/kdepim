#ifndef FILTERACTIONSETTRANSPORTTEST_H
#define FILTERACTIONSETTRANSPORTTEST_H

#include <QObject>

class FilterActionSetTransportTest : public QObject
{
    Q_OBJECT
public:
    explicit FilterActionSetTransportTest(QObject *parent = Q_NULLPTR);
    ~FilterActionSetTransportTest();

private Q_SLOTS:
    void shouldBeEmpty();
    void shouldHaveDefaultValue();
    void shouldHaveRequiredPart();
    void shouldHaveSieveRequires();
};

#endif // FILTERACTIONSETTRANSPORTTEST_H
