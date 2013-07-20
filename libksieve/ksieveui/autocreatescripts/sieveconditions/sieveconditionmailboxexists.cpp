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


#include "sieveconditionmailboxexists.h"

#include <KLocale>
#include <KLineEdit>

#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>

using namespace KSieveUi;
SieveConditionMailboxExists::SieveConditionMailboxExists(QObject *parent)
    : SieveCondition(QLatin1String("mailboxexist"), i18n("Mailbox exist"), parent)
{
}

SieveCondition *SieveConditionMailboxExists::newAction()
{
    return new SieveConditionMailboxExists;
}

QWidget *SieveConditionMailboxExists::createParamWidget( QWidget *parent ) const
{
    QWidget *w = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    w->setLayout(lay);

    KLineEdit *edit = new KLineEdit;
    edit->setClearButtonShown(true);
    lay->addWidget(edit);
    edit->setObjectName(QLatin1String("edit"));

    return w;
}

QString SieveConditionMailboxExists::code(QWidget *w) const
{
    KLineEdit *edit = w->findChild<KLineEdit*>( QLatin1String("edit"));
    const QString editValue = edit->text();
    return QString::fromLatin1("mailboxexists \"%1\"").arg(editValue);
}

QStringList SieveConditionMailboxExists::needRequires(QWidget *) const
{
    return QStringList() << QLatin1String("mailbox");
}

bool SieveConditionMailboxExists::needCheckIfServerHasCapability() const
{
    return true;
}

QString SieveConditionMailboxExists::serverNeedsCapability() const
{
    return QLatin1String("mailbox");
}

QString SieveConditionMailboxExists::help() const
{
    return i18n("The \"mailboxexists\" test is true if all mailboxes listed in the \"mailbox-names\" argument exist in the mailstore, and each allows the user in whose context the Sieve script runs to \"deliver\" messages into it.");
}

void SieveConditionMailboxExists::setParamWidgetValue(const QDomElement &element, QWidget *parent ) const
{

}

#include "sieveconditionmailboxexists.moc"
