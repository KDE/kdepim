/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "grantleeprint.h"
#include "contactgrantleeprintobject.h"

#include "formatter/grantleecontactutils.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

using namespace KAddressBookGrantlee;

GrantleePrint::GrantleePrint(QObject *parent)
    : QObject(parent)
{
    mEngine = new Grantlee::Engine;
}

GrantleePrint::GrantleePrint(const QString &themePath, QObject *parent)
    : QObject(parent)
{
    mEngine = new Grantlee::Engine;
    mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);

    changeGrantleePath(themePath);
}

GrantleePrint::~GrantleePrint()
{
    delete mEngine;
}

void GrantleePrint::refreshTemplate()
{
    mSelfcontainedTemplate = mEngine->loadByName(QStringLiteral("theme.html"));
    if (mSelfcontainedTemplate->error()) {
        mErrorMessage += mSelfcontainedTemplate->errorString() + QLatin1String("<br>");
    }
}

void GrantleePrint::changeGrantleePath(const QString &path)
{
    if (!mTemplateLoader) {
        mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);
    }
    mTemplateLoader->setTemplateDirs(QStringList() << path);
    mEngine->addTemplateLoader(mTemplateLoader);

    refreshTemplate();
}

void GrantleePrint::setContent(const QString &content)
{
    mSelfcontainedTemplate = mEngine->newTemplate(content, QStringLiteral("content"));
    if (mSelfcontainedTemplate->error()) {
        mErrorMessage = mSelfcontainedTemplate->errorString() + QLatin1String("<br>");
    }
}

QString GrantleePrint::contactsToHtml(const KContacts::Addressee::List &contacts)
{
    if (!mErrorMessage.isEmpty()) {
        return mErrorMessage;
    }

    if (contacts.isEmpty()) {
        return QString();
    }
    QVariantList contactsList;
    QList<ContactGrantleePrintObject *> lst;
    const int numberContacts(contacts.count());
    lst.reserve(numberContacts);
    contactsList.reserve(numberContacts);
    Q_FOREACH (const KContacts::Addressee &address, contacts) {
        ContactGrantleePrintObject *contactPrintObject = new ContactGrantleePrintObject(address);
        lst.append(contactPrintObject);
        contactsList << QVariant::fromValue(static_cast<QObject *>(contactPrintObject));
    }
    QVariantHash mapping;
    QVariantHash contactI18n;
    GrantleeContactUtils grantleeContactUtil;
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("birthdayi18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("anniversaryi18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("emailsi18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("websitei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("blogUrli18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("addressBookNamei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("notei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("departmenti18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("Professioni18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("officei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("manageri18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("assistanti18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("spousei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("imAddressi18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("latitudei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("longitudei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("organizationi18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("titlei18n"));
    grantleeContactUtil.insertVariableToQVariantHash(contactI18n, QStringLiteral("nextcontacti18n"));
    mapping.insert(QStringLiteral("contacti18n"), contactI18n);

    Grantlee::Context context(mapping);
    context.insert(QStringLiteral("contacts"), contactsList);
    const QString content = mSelfcontainedTemplate->render(&context);
    qDeleteAll(lst);
    return content;
}

