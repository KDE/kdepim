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

#include "sieveactionbreak.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QLabel>

using namespace KSieveUi;
SieveActionBreak::SieveActionBreak(QObject *parent)
    : SieveAction(QLatin1String("break"), i18n("Break"), parent)
{
}

SieveAction* SieveActionBreak::newAction()
{
    return new SieveActionBreak;
}

QWidget *SieveActionBreak::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    QLabel *lab = new QLabel(i18n("Name (optional):"));
    lay->addWidget(lab);

    KLineEdit *subject = new KLineEdit;
    subject->setObjectName(QLatin1String("name"));
    lay->addWidget(subject);
    return w;
}

void SieveActionBreak::setParamWidgetValue(QWidget *w ) const
{
    const KLineEdit *name = w->findChild<KLineEdit*>(QLatin1String("name"));
    //name->setText();
}

QString SieveActionBreak::code(QWidget *w) const
{
    QString result;
    const KLineEdit *name = w->findChild<KLineEdit*>(QLatin1String("name"));
    const QString nameStr = name->text();
    if (!nameStr.isEmpty()) {
        result = QString::fromLatin1(":name \"%1\"").arg(nameStr);
    }
    return QString::fromLatin1("break %1;").arg(result);
}

QString SieveActionBreak::help() const
{
    return i18n("The break command terminates the closest enclosing loop.");
}

QStringList SieveActionBreak::needRequires(QWidget */*parent*/) const
{
    return QStringList() << QLatin1String("foreverypart");
}

bool SieveActionBreak::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionBreak::serverNeedsCapability() const
{
    return QLatin1String("foreverypart");
}


#include "sieveactionbreak.moc"
