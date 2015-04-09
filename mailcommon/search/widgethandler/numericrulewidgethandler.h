#ifndef NUMERICRULEWIDGETHANDLER_H
#define NUMERICRULEWIDGETHANDLER_H

#include "interfaces/rulewidgethandler.h"

namespace MailCommon
{
class NumericRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
public:
    NumericRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~NumericRuleWidgetHandler()
    {
    }

    QWidget *createFunctionWidget(int number,
                                  QStackedWidget *functionStack,
                                  const QObject *receiver , bool isBalooSearch) const Q_DECL_OVERRIDE;

    QWidget *createValueWidget(int number,
                               QStackedWidget *valueStack,
                               const QObject *receiver) const Q_DECL_OVERRIDE;

    SearchRule::Function function(const QByteArray &field,
                                  const QStackedWidget *functionStack) const Q_DECL_OVERRIDE;

    QString value(const QByteArray &field,
                  const QStackedWidget *functionStack,
                  const QStackedWidget *valueStack) const Q_DECL_OVERRIDE;

    QString prettyValue(const QByteArray &field,
                        const QStackedWidget *functionStack,
                        const QStackedWidget *valueStack) const Q_DECL_OVERRIDE;

    bool handlesField(const QByteArray &field) const Q_DECL_OVERRIDE;

    void reset(QStackedWidget *functionStack,
               QStackedWidget *valueStack) const Q_DECL_OVERRIDE;

    bool setRule(QStackedWidget *functionStack,
                 QStackedWidget *valueStack,
                 const SearchRule::Ptr rule, bool isBalooSearch) const Q_DECL_OVERRIDE;

    bool update(const QByteArray &field,
                QStackedWidget *functionStack,
                QStackedWidget *valueStack) const Q_DECL_OVERRIDE;

private:
    SearchRule::Function currentFunction(const QStackedWidget *functionStack) const;
    QString currentValue(const QStackedWidget *valueStack) const;
};
}
#endif // NUMERICRULEWIDGETHANDLER_H
