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


#include "sieveactionextracttext.h"

#include <KLocale>
#include <KLineEdit>

#include <QLabel>
#include <QHBoxLayout>
#include <QSpinBox>

using namespace KSieveUi;
SieveActionExtractText::SieveActionExtractText(QObject *parent)
    : SieveAction(QLatin1String("extracttext"), i18n("Extract Text"), parent)
{
}

SieveAction* SieveActionExtractText::newAction()
{
    return new SieveActionExtractText;
}

QWidget *SieveActionExtractText::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Number of characters:"));
    lay->addWidget(lab);

    QSpinBox *nbCharacters = new QSpinBox;
    nbCharacters->setMinimum(1);
    nbCharacters->setMaximum(99999);
    nbCharacters->setObjectName(QLatin1String("numberOfCharacters"));
    lay->addWidget(nbCharacters);

    lab = new QLabel(i18n("Stored in variable name:"));
    lay->addWidget(lab);

    KLineEdit *variableName = new KLineEdit;
    variableName->setObjectName(QLatin1String("variablename"));
    lay->addWidget(variableName);

    return w;
}

void SieveActionExtractText::setParamWidgetValue(QWidget *parent ) const
{

}

QString SieveActionExtractText::code(QWidget *w) const
{
    const QSpinBox *numberOfCharacters = w->findChild<QSpinBox*>(QLatin1String("numberOfCharacters"));
    const QString numberOfCharactersStr = QString::number(numberOfCharacters->value());

    const KLineEdit *variableName = w->findChild<KLineEdit*>(QLatin1String("variablename"));
    const QString variableNameStr = variableName->text();

    const QString result = QString::fromLatin1("extracttext :first %1 \"%2\";").arg(numberOfCharactersStr).arg(variableNameStr);
    return result;
}


QStringList SieveActionExtractText::needRequires(QWidget *parent) const
{
    return QStringList() << QLatin1String("extracttext");
}

bool SieveActionExtractText::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionExtractText::serverNeedsCapability() const
{
    return QLatin1String("extracttext");
}

QString SieveActionExtractText::help() const
{
    return i18n("The \"extracttext\" action may be used within the context of a \"foreverypart\" loop and is used to store text into a variable");
}

#include "sieveactionextracttext.moc"
