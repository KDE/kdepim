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
#include "autocreatescripts/autocreatescriptdialog.h"

#include <KLocale>
#include <KLineEdit>

#include <QCheckBox>
#include <QHBoxLayout>

using namespace KSieveUi;
SieveActionFileInto::SieveActionFileInto(QObject *parent)
    : SieveAction(QLatin1String("fileinto"), i18n("File Into"), parent)
{
    mHasCopySupport = AutoCreateScriptDialog::sieveCapabilities().contains(QLatin1String("copy"));
}

SieveAction* SieveActionFileInto::newAction()
{
  return new SieveActionFileInto;
}

QString SieveActionFileInto::code(QWidget *w) const
{
    QString result = QString::fromLatin1("fileinto ");
    const KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("fileintolineedit") );
    const QString text = edit->text();
    if (mHasCopySupport) {
        const QCheckBox *copy = w->findChild<QCheckBox*>( QLatin1String("copy") );
        if (copy->isChecked())
            result += QLatin1String(":copy ");
    }
    return result + QString::fromLatin1("\"%1\";").arg(text);
}

QWidget *SieveActionFileInto::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    if (mHasCopySupport) {
        QCheckBox *copy = new QCheckBox(i18n("Keep a copy"));
        copy->setObjectName(QLatin1String("copy"));
        lay->addWidget(copy);
    }
    //TODO improve it.
    KLineEdit *edit = new KLineEdit;
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("fileintolineedit"));
    return w;
}

QStringList SieveActionFileInto::needRequires() const
{
    return QStringList()<<QLatin1String("fileinto") << QLatin1String("copy");
}

bool SieveActionFileInto::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveActionFileInto::serverNeedsCapability() const
{
    return QLatin1String("fileinto");
}

#include "sieveactionfileinto.moc"
