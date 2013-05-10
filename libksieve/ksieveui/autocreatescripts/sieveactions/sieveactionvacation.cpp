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
#include "widgets/multilineedit.h"

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

    QLabel *lab = new QLabel(i18n("day:"));
    lay->addWidget(lab);

    QSpinBox *day = new QSpinBox;
    day->setMinimum(7);
    day->setObjectName(QLatin1String("day"));
    lay->addWidget(day);

    lab = new QLabel(i18n("Vacation reason:"));
    lay->addWidget(lab);

    MultiLineEdit *text = new MultiLineEdit;
    text->setObjectName(QLatin1String("text"));
    lay->addWidget(text);

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

    return w;
}

QString SieveActionVacation::code(QWidget *w) const
{
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
        result += QString::fromLatin1(" :days %1").arg(dayStr);
    if (!textStr.isEmpty())
        result += QString::fromLatin1(" :text %1").arg(textStr);
    if (!subjectStr.isEmpty())
        result += QString::fromLatin1(" :subject %1").arg(subjectStr);
    if (!addressesStr.isEmpty())
        result += QString::fromLatin1(" :addresses %1").arg(addressesStr);
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

QStringList SieveActionVacation::needRequires() const
{
    return QStringList() <<QLatin1String("vacation");
}


#include "sieveactionvacation.moc"

