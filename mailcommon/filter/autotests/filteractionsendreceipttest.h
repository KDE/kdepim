#ifndef FILTERACTIONSENDRECEIPTTEST_H
#define FILTERACTIONSENDRECEIPTTEST_H

#include <QObject>

class FilterActionSendReceiptTest : public QObject
{
    Q_OBJECT
public:
    explicit FilterActionSendReceiptTest(QObject *parent = Q_NULLPTR);
private Q_SLOTS:
    void shouldBeNotEmpty();
    void shouldRequiresPart();
    void shouldSieveRequires();
};

#endif // FILTERACTIONSENDRECEIPTTEST_H
