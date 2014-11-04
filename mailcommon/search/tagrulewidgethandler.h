/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef TAGRULEWIDGETHANDLER_H
#define TAGRULEWIDGETHANDLER_H

#include "interfaces/rulewidgethandler.h"

namespace MailCommon
{
class TagRuleWidgetHandler : public MailCommon::RuleWidgetHandler
{
public:
    TagRuleWidgetHandler() : MailCommon::RuleWidgetHandler()
    {
    }

    ~TagRuleWidgetHandler()
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
                 const SearchRule::Ptr rule , bool isBalooSearch) const Q_DECL_OVERRIDE;

    bool update(const QByteArray &field,
                QStackedWidget *functionStack,
                QStackedWidget *valueStack) const Q_DECL_OVERRIDE;
};

}

#endif // TAGRULEWIDGETHANDLER_H
