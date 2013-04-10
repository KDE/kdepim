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

#include "sieveactionreject.h"

#include <KLineEdit>
#include <KLocale>

#include <QLabel>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionReject::SieveActionReject(QObject *parent)
    : SieveAction(QLatin1String("reject"), i18n("Reject"), parent)
{
}

SieveAction *SieveActionReject::newAction()
{
    return new SieveActionReject;
}

QWidget *SieveActionReject::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    w->setLayout(lay);
    QLabel *lab = new QLabel(i18n("text:"));
    lay->addWidget(lab);

    KLineEdit *edit = new KLineEdit;
    edit->setObjectName( QLatin1String("RejectText") );
    lay->addWidget(edit);
    return w;
}

QString SieveActionReject::code(QWidget *w) const
{
    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("RejectText") );
    const QString text = edit->text();

    return QString::fromLatin1("reject \"%1\";").arg(text);
}

QStringList SieveActionReject::needRequires() const
{
    return QStringList() <<QLatin1String("reject");
}


#include "sieveactionreject.moc"
