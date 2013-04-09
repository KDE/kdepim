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

#include "sieveactionfileinto.h"
#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionFileInto::SieveActionFileInto(QObject *parent)
    : SieveAction(QLatin1String("fileinto"), i18n("File Into"), parent)
{
}

SieveAction* SieveActionFileInto::newAction()
{
  return new SieveActionFileInto;
}

QString SieveActionFileInto::code(QWidget *) const
{
    //TODO
    return QString();
}

QWidget *SieveActionFileInto::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    w->setLayout(lay);

    //TODO improve it.
    KLineEdit *edit = new KLineEdit;
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("fileintolineedit"));
    return w;
}

QStringList SieveActionFileInto::needRequires() const
{
    return QStringList()<<QLatin1String("fileinto");
}


#include "sieveactionfileinto.moc"
