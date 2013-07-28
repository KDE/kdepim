/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "autocreatescripts/autocreatescriptdialog.h"
#include "autocreatescripts/autocreatescriptutil_p.h"
#include "widgets/multilineedit.h"
#include "widgets/selectvacationcombobox.h"


#include <KLocale>
#include <KLineEdit>

#include <KTextEdit>

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

using namespace KSieveUi;

SieveActionVacation::SieveActionVacation(QObject *parent)
    : SieveAction(QLatin1String("vacation"), i18n("Vacation"), parent)
{
    mHasVacationSecondsSupport = AutoCreateScriptDialog::sieveCapabilities().contains(QLatin1String("vacation-seconds"));
}

SieveAction* SieveActionVacation::newAction()
{
    return new SieveActionVacation;
}

QWidget *SieveActionVacation::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setAlignment(Qt::AlignBottom);
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = 0;
    if (mHasVacationSecondsSupport) {
        SelectVacationComboBox *vacation = new SelectVacationComboBox;
        vacation->setObjectName(QLatin1String("vacationcombobox"));
        lay->addWidget(vacation);
    } else {
        lab = new QLabel(i18n("day:"));
        lay->addWidget(lab);
    }

    QSpinBox *day = new QSpinBox;
    day->setMinimum(1);
    day->setObjectName(QLatin1String("day"));
    lay->addWidget(day);


    lab = new QLabel(i18n("Message subject:"));
    lay->addWidget(lab);

    KLineEdit *subject = new KLineEdit;
    subject->setObjectName(QLatin1String("subject"));
    lay->addWidget(subject);

    lab = new QLabel(i18n("Additional email:"));
    lay->addWidget(lab);

    KLineEdit *addresses = new KLineEdit;
    addresses->setObjectName(QLatin1String("addresses"));
    lay->addWidget(addresses);

    lab = new QLabel(i18n("Vacation reason:"));
    lay->addWidget(lab);

    MultiLineEdit *text = new MultiLineEdit;
    text->setObjectName(QLatin1String("text"));
    lay->addWidget(text);

    return w;
}

QString SieveActionVacation::code(QWidget *w) const
{
    QString vacationTypeStr = QLatin1String(":days");
    if (mHasVacationSecondsSupport) {
        const SelectVacationComboBox *vacationcombobox = w->findChild<SelectVacationComboBox*>(QLatin1String("vacationcombobox"));
        vacationTypeStr = vacationcombobox->code();
    }
    const QSpinBox *day = w->findChild<QSpinBox*>( QLatin1String("day") );
    const QString dayStr = QString::number(day->value());

    const MultiLineEdit *text = w->findChild<MultiLineEdit*>( QLatin1String("text") );
    const QString textStr = text->toPlainText();

    const KLineEdit *subject = w->findChild<KLineEdit*>( QLatin1String("subject") );
    const QString subjectStr = subject->text();

    const KLineEdit *addresses = w->findChild<KLineEdit*>( QLatin1String("addresses") );
    const QString addressesStr = addresses->text();
    QString result = QString::fromLatin1("vacation");
    if (!dayStr.isEmpty())
        result += QString::fromLatin1(" %1 %2").arg(vacationTypeStr).arg(dayStr);
    if (!subjectStr.isEmpty())
        result += QString::fromLatin1(" :subject \"%1\"").arg(subjectStr);
    if (!addressesStr.isEmpty())
        result += QString::fromLatin1(" :addresses %1").arg(AutoCreateScriptUtil::createAddressList(addressesStr));
    if (!textStr.isEmpty())
        result += QString::fromLatin1(" text:%1").arg(AutoCreateScriptUtil::createMultiLine(textStr));
    else
        result += QLatin1Char(';'); //Be sure to have ";"
    return result;
}

QString SieveActionVacation::serverNeedsCapability() const
{
    return QLatin1String("vacation");
}

bool SieveActionVacation::needCheckIfServerHasCapability() const
{
    return true;
}

QStringList SieveActionVacation::needRequires(QWidget *) const
{
    QStringList lst;
    if (mHasVacationSecondsSupport) {
        lst << QLatin1String("vacation-seconds");
    }
    lst << QLatin1String("vacation");
    return lst;
}

QString SieveActionVacation::help() const
{
    QString helpStr = i18n("The \"vacation\" action implements a vacation autoresponder similar to the vacation command available under many versions of Unix. Its purpose is to provide correspondents with notification that the user is away for an extended period of time and that they should not expect quick responses.");
    if (mHasVacationSecondsSupport) {
        helpStr = QLatin1Char('\n') + i18n("Through the \":days\" parameter, it limits the number of auto-replies to the same sender to one per [n] days, for a specified number of days. But there are cases when one needs more granularity, if one would like to generate \"vacation\" replies more frequently.");
        helpStr = QLatin1Char('\n') + i18n("This extension defines a \":seconds\" parameter to provide more granularity for such situations.");
    }
    return helpStr;
}

#include "sieveactionvacation.moc"

