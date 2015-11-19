/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "sieveactionvacation.h"
#include "editor/sieveeditorutil.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/multilineedit.h"
#include "widgets/selectvacationcombobox.h"

#include <KLocalizedString>
#include <QLineEdit>

#include <QLabel>
#include <QSpinBox>
#include "libksieve_debug.h"
#include <QDomNode>
#include <QGridLayout>

using namespace KSieveUi;

SieveActionVacation::SieveActionVacation(QObject *parent)
    : SieveAction(QStringLiteral("vacation"), i18n("Vacation"), parent)
{
    mHasVacationSecondsSupport = SieveEditorGraphicalModeWidget::sieveCapabilities().contains(QStringLiteral("vacation-seconds"));
}

SieveAction *SieveActionVacation::newAction()
{
    return new SieveActionVacation;
}

QWidget *SieveActionVacation::createParamWidget(QWidget *parent) const
{
    QWidget *w = new QWidget(parent);
    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    w->setLayout(grid);

    QLabel *lab = Q_NULLPTR;
    if (mHasVacationSecondsSupport) {
        SelectVacationComboBox *vacation = new SelectVacationComboBox;
        vacation->setObjectName(QStringLiteral("vacationcombobox"));
        connect(vacation, &SelectVacationComboBox::valueChanged, this, &SieveActionVacation::valueChanged);
        grid->addWidget(vacation, 0, 0);
    } else {
        lab = new QLabel(i18n("day:"));
        grid->addWidget(lab, 0, 0);
    }

    QSpinBox *day = new QSpinBox;
    day->setMinimum(1);
    day->setMaximum(999);
    day->setObjectName(QStringLiteral("day"));
    connect(day, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SieveActionVacation::valueChanged);
    grid->addWidget(day, 0, 1);

    lab = new QLabel(i18n("Message subject:"));
    grid->addWidget(lab, 1, 0);

    QLineEdit *subject = new QLineEdit;
    subject->setObjectName(QStringLiteral("subject"));
    connect(subject, &QLineEdit::textChanged, this, &SieveActionVacation::valueChanged);
    grid->addWidget(subject, 1, 1);

    lab = new QLabel(i18n("Additional email:"));
    grid->addWidget(lab, 2, 0);

    QLineEdit *addresses = new QLineEdit;
    addresses->setObjectName(QStringLiteral("addresses"));
    connect(addresses, &QLineEdit::textChanged, this, &SieveActionVacation::valueChanged);
    grid->addWidget(addresses, 2, 1);

    lab = new QLabel(i18n("Vacation reason:"));
    grid->addWidget(lab, 3, 0);

    MultiLineEdit *text = new MultiLineEdit;
    connect(text, &MultiLineEdit::valueChanged, this, &SieveActionVacation::valueChanged);
    text->setObjectName(QStringLiteral("text"));
    grid->addWidget(text, 3, 1);

    return w;
}

bool SieveActionVacation::setParamWidgetValue(const QDomElement &element, QWidget *w, QString &error)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull()) {
            const QString tagName = e.tagName();
            if (tagName == QLatin1String("tag")) {
                const QString tagValue = e.text();
                if (tagValue == QLatin1String("seconds")) {
                    if (mHasVacationSecondsSupport) {
                        SelectVacationComboBox *vacationcombobox = w->findChild<SelectVacationComboBox *>(QStringLiteral("vacationcombobox"));
                        vacationcombobox->setCode(AutoCreateScriptUtil::tagValue(tagValue), name(), error);
                    } else {
                        serverDoesNotSupportFeatures(QStringLiteral("seconds"), error);
                    }
                } else if (tagValue == QLatin1String("days")) {
                    //Nothing wait num tag for it.
                } else if (tagValue == QLatin1String("addresses")) {
                    QLineEdit *addresses = w->findChild<QLineEdit *>(QStringLiteral("addresses"));
                    addresses->setText(AutoCreateScriptUtil::strValue(e));
                } else if (tagValue == QLatin1String("subject")) {
                    QLineEdit *subject = w->findChild<QLineEdit *>(QStringLiteral("subject"));
                    subject->setText(AutoCreateScriptUtil::strValue(e));
                } else {
                    unknowTagValue(tagValue, error);
                    qCDebug(LIBKSIEVE_LOG) << "SieveActionVacation::setParamWidgetValue unknown tagValue :" << tagValue;
                }
            } else if (tagName == QLatin1String("num"))  {
                QSpinBox *day = w->findChild<QSpinBox *>(QStringLiteral("day"));
                day->setValue(e.text().toInt());
            } else if (tagName == QLatin1String("str")) {
                MultiLineEdit *text = w->findChild<MultiLineEdit *>(QStringLiteral("text"));
                text->setPlainText(e.text());
            } else if (tagName == QLatin1String("list")) {
                QLineEdit *addresses = w->findChild<QLineEdit *>(QStringLiteral("addresses"));
                addresses->setText(AutoCreateScriptUtil::listValueToStr(e));
            } else if (tagName == QLatin1String("crlf")) {
                //nothing
            } else if (tagName == QLatin1String("comment")) {
                //implement in the future ?
            } else {
                unknownTag(tagName, error);
                qCDebug(LIBKSIEVE_LOG) << " SieveActionVacation::setParamWidgetValue unknown tagName " << tagName;
            }
        }
        node = node.nextSibling();
    }
    return true;

}

QString SieveActionVacation::code(QWidget *w) const
{
    QString vacationTypeStr = QStringLiteral(":days");
    if (mHasVacationSecondsSupport) {
        const SelectVacationComboBox *vacationcombobox = w->findChild<SelectVacationComboBox *>(QStringLiteral("vacationcombobox"));
        vacationTypeStr = vacationcombobox->code();
    }
    const QSpinBox *day = w->findChild<QSpinBox *>(QStringLiteral("day"));
    const QString dayStr = QString::number(day->value());

    const MultiLineEdit *text = w->findChild<MultiLineEdit *>(QStringLiteral("text"));
    const QString textStr = text->toPlainText();

    const QLineEdit *subject = w->findChild<QLineEdit *>(QStringLiteral("subject"));
    const QString subjectStr = subject->text();

    const QLineEdit *addresses = w->findChild<QLineEdit *>(QStringLiteral("addresses"));
    const QString addressesStr = addresses->text();
    QString result = QStringLiteral("vacation");
    if (!dayStr.isEmpty()) {
        result += QStringLiteral(" %1 %2").arg(vacationTypeStr, dayStr);
    }
    if (!subjectStr.isEmpty()) {
        result += QStringLiteral(" :subject \"%1\"").arg(subjectStr);
    }
    if (!addressesStr.isEmpty()) {
        result += QStringLiteral(" :addresses %1").arg(AutoCreateScriptUtil::createAddressList(addressesStr, false));
    }
    if (!textStr.isEmpty()) {
        result += QStringLiteral(" text:%1").arg(AutoCreateScriptUtil::createMultiLine(textStr));
    } else {
        result += QLatin1Char(';');    //Be sure to have ";"
    }
    return result;
}

QString SieveActionVacation::serverNeedsCapability() const
{
    return QStringLiteral("vacation");
}

bool SieveActionVacation::needCheckIfServerHasCapability() const
{
    return true;
}

QStringList SieveActionVacation::needRequires(QWidget *) const
{
    QStringList lst;
    if (mHasVacationSecondsSupport) {
        lst << QStringLiteral("vacation-seconds");
    }
    lst << QStringLiteral("vacation");
    return lst;
}

QString SieveActionVacation::help() const
{
    QString helpStr = i18n("The \"vacation\" action implements a vacation autoresponder similar to the vacation command available under many versions of Unix. Its purpose is to provide correspondents with notification that the user is away for an extended period of time and that they should not expect quick responses.");
    if (mHasVacationSecondsSupport) {
        helpStr = QLatin1Char('\n') + i18n("Through the \":days\" parameter, it limits the number of auto-replies to the same sender to one per [n] days, for a specified number of days. But there are cases when one needs more granularity, if one would like to generate \"vacation\" replies more frequently.");
        helpStr += QLatin1Char('\n') + i18n("This extension defines a \":seconds\" parameter to provide more granularity for such situations.");
    }
    return helpStr;
}

QUrl SieveActionVacation::href() const
{
    return SieveEditorUtil::helpUrl(SieveEditorUtil::strToVariableName(name()));
}
