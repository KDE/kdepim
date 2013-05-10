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

    lab = new QLabel(i18n("Text:"));
    lay->addWidget(lab);

    MultiLineEdit *text = new MultiLineEdit;
    text->setObjectName(QLatin1String("text"));
    lay->addWidget(text);

    //TODO
    lay->addWidget(day);

    return w;
}

QString SieveActionVacation::code(QWidget *w) const
{
    const QSpinBox *day = w->findChild<QSpinBox*>( QLatin1String("day") );
    const QString dayStr = QString::number(day->value());

    const MultiLineEdit *text = w->findChild<MultiLineEdit*>( QLatin1String("text") );
    const QString textStr = text->toPlainText();

    //TODO
    return QString::fromLatin1("vacation :days %1 text: %2").arg(dayStr).arg(textStr);
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

