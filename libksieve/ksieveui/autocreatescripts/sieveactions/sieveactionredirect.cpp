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

#include "sieveactionredirect.h"
#include "autocreatescripts/autocreatescriptdialog.h"
#include "widgets/addresslineedit.h"

#include <KLocale>
#include <KLineEdit>

#include <QHBoxLayout>
#include <QCheckBox>

using namespace KSieveUi;

SieveActionRedirect::SieveActionRedirect(QObject *parent)
    : SieveAction(QLatin1String("redirect"), i18n("Redirect"), parent)
{
    mHasCopySupport = AutoCreateScriptDialog::sieveCapabilities().contains(QLatin1String("copy"));
}

SieveAction *SieveActionRedirect::newAction()
{
    return new SieveActionRedirect;
}

QWidget *SieveActionRedirect::createParamWidget( QWidget *parent ) const
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
    AddressLineEdit *edit = new AddressLineEdit;
    edit->setObjectName(QLatin1String("RedirectEdit"));
    lay->addWidget(edit);
    return w;
}

QString SieveActionRedirect::code(QWidget *w) const
{
    QString result = QLatin1String("redirect ");
    const KLineEdit *edit = w->findChild<AddressLineEdit*>( QLatin1String("RedirectEdit") );
    const QString text = edit->text();

    if (mHasCopySupport) {
        const QCheckBox *copy = w->findChild<QCheckBox*>( QLatin1String("copy") );
        if (copy->isChecked())
            result += QLatin1String(":copy ");
    }
    return result + QString::fromLatin1("\"%1\";").arg(text);
}

QStringList SieveActionRedirect::needRequires() const
{
    if (mHasCopySupport) {
        return QStringList() <<QLatin1String("copy");
    }
    return QStringList();
}

QString SieveActionRedirect::help() const
{
    //TODO add copy info
    return i18n("The \"redirect\" action is used to send the message to another user at a supplied address, as a mail forwarding feature does.  The \"redirect\" action makes no changes to the message body or existing headers, but it may add new headers.");
}


#include "sieveactionredirect.moc"
