#ifndef FILTERACTIONSENDRECEIPTTEST_H
#define FILTERACTIONSENDRECEIPTTEST_H

#include <QObject>

class FilterActionSendReceiptTest : public QObject
{
    Q_OBJECT
public:
    explicit FilterActionSendReceiptTest(QObject *parent = 0);
private Q_SLOTS:
    void shouldBeNotEmpty();
    void shouldRequiresPart();

};

#endif // FILTERACTIONSENDRECEIPTTEST_H
