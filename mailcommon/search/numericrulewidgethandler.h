#ifndef NUMERICRULEWIDGETHANDLER_H
#define NUMERICRULEWIDGETHANDLER_H

#include "interfaces/rulewidgethandler.h"

namespace MailCommon {
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
                                   const QObject *receiver , bool isBalooSearch) const;

    QWidget *createValueWidget( int number,
                                QStackedWidget *valueStack,
                                const QObject *receiver ) const;

    SearchRule::Function function( const QByteArray & field,
                                   const QStackedWidget *functionStack ) const;

    QString value( const QByteArray & field,
                   const QStackedWidget *functionStack,
                   const QStackedWidget *valueStack ) const;

    QString prettyValue( const QByteArray & field,
                         const QStackedWidget *functionStack,
                         const QStackedWidget *valueStack ) const;

    bool handlesField( const QByteArray & field ) const;

    void reset( QStackedWidget *functionStack,
                QStackedWidget *valueStack ) const;

    bool setRule( QStackedWidget *functionStack,
                  QStackedWidget *valueStack,
                  const SearchRule::Ptr rule, bool isBalooSearch ) const;

    bool update( const QByteArray & field,
                 QStackedWidget *functionStack,
                 QStackedWidget *valueStack ) const;

private:
    SearchRule::Function currentFunction( const QStackedWidget *functionStack ) const;
    QString currentValue( const QStackedWidget *valueStack ) const;
};
}
#endif // NUMERICRULEWIDGETHANDLER_H
