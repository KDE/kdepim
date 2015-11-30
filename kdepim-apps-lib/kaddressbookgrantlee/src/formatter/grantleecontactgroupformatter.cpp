/*
  This file is part of KAddressBook.

  Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "grantleecontactgroupformatter.h"

#include "grantleetheme/grantleetheme.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

#include <AkonadiCore/Item>
#include <Akonadi/Contact/ContactGroupExpandJob>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

#include <KColorScheme>

using namespace KAddressBookGrantlee;

class Q_DECL_HIDDEN GrantleeContactGroupFormatter::Private
{
public:
    Private()
    {
        mEngine = new Grantlee::Engine;

        mTemplateLoader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);
    }

    ~Private()
    {
        delete mEngine;
    }

    void changeGrantleePath(const QString &path)
    {
        mTemplateLoader->setTemplateDirs(QStringList() << path);
        mEngine->addTemplateLoader(mTemplateLoader);

        mSelfcontainedTemplate = mEngine->loadByName(QStringLiteral("contactgroup.html"));
        if (mSelfcontainedTemplate->error()) {
            mErrorMessage += mSelfcontainedTemplate->errorString() + QStringLiteral("<br>");
        }

        mEmbeddableTemplate = mEngine->loadByName(QStringLiteral("contactgroup_embedded.html"));
        if (mEmbeddableTemplate->error()) {
            mErrorMessage += mEmbeddableTemplate->errorString() + QStringLiteral("<br>");
        }
    }

    QVector<QObject *> mObjects;
    Grantlee::Engine *mEngine;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> mTemplateLoader;
    Grantlee::Template mSelfcontainedTemplate;
    Grantlee::Template mEmbeddableTemplate;
    QString mErrorMessage;
};

GrantleeContactGroupFormatter::GrantleeContactGroupFormatter()
    : d(new Private)
{
}

GrantleeContactGroupFormatter::~GrantleeContactGroupFormatter()
{
    delete d;
}

void GrantleeContactGroupFormatter::setAbsoluteThemePath(const QString &path)
{
    d->changeGrantleePath(path);
}

void GrantleeContactGroupFormatter::setGrantleeTheme(const GrantleeTheme::Theme &theme)
{
    d->changeGrantleePath(theme.absolutePath());
}

inline static void setHashField(QVariantHash &hash, const QString &name, const QString &value)
{
    if (!value.isEmpty()) {
        hash.insert(name, value);
    }
}

static QVariantHash memberHash(const KContacts::ContactGroup::Data &data)
{
    QVariantHash memberObject;

    setHashField(memberObject, QStringLiteral("name"), data.name());
    setHashField(memberObject, QStringLiteral("email"), data.email());

    KContacts::Addressee contact;
    contact.setFormattedName(data.name());
    contact.insertEmail(data.email());

    const QString emailLink = QStringLiteral("<a href=\"mailto:") +
                              QString::fromLatin1(QUrl::toPercentEncoding(contact.fullEmail())) +
                              QStringLiteral("\">%1</a>").arg(contact.preferredEmail());

    setHashField(memberObject, QStringLiteral("emailLink"), emailLink);

    return memberObject;
}

QString GrantleeContactGroupFormatter::toHtml(HtmlForm form) const
{
    if (!d->mErrorMessage.isEmpty()) {
        return d->mErrorMessage;
    }

    KContacts::ContactGroup group;
    const Akonadi::Item localItem = item();
    if (localItem.isValid() && localItem.hasPayload<KContacts::ContactGroup>()) {
        group = localItem.payload<KContacts::ContactGroup>();
    } else {
        group = contactGroup();
    }

    if (group.name().isEmpty() && group.count() == 0) {   // empty group
        return QString();
    }

    if (group.contactReferenceCount() != 0) {
        // we got a contact group with unresolved references -> we have to resolve
        // it ourself.  this shouldn't be the normal case, actually the calling
        // code should pass in an already resolved contact group
        Akonadi::ContactGroupExpandJob *job = new Akonadi::ContactGroupExpandJob(group);
        if (job->exec()) {
            group.removeAllContactData();
            foreach (const KContacts::Addressee &contact, job->contacts()) {
                group.append(KContacts::ContactGroup::Data(contact.realName(), contact.preferredEmail()));
            }
        }
    }

    QVariantHash contactGroupObject;

    setHashField(contactGroupObject, QStringLiteral("name"), group.name());

    // Group members
    QVariantList members;
    const int numberOfData(group.dataCount());
    members.reserve(numberOfData);
    for (uint i = 0; i < numberOfData; ++i) {
        members << memberHash(group.data(i));
    }

    contactGroupObject.insert(QStringLiteral("members"), members);

    // Additional fields
    QVariantList fields;
    foreach (const QVariantMap &field, additionalFields()) {
        QVariantHash fieldObject;
        setHashField(fieldObject, QStringLiteral("key"),
                     field.value(QStringLiteral("key")).toString());

        setHashField(fieldObject, QStringLiteral("title"),
                     field.value(QStringLiteral("title")).toString());

        setHashField(fieldObject, QStringLiteral("value"),
                     field.value(QStringLiteral("value")).toString());

        fields << fieldObject;
    }

    contactGroupObject.insert(QStringLiteral("additionalFields"), fields);

    QVariantHash colorsObject;
    colorsObject.insert(
        QStringLiteral("linkColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).foreground().color().name());

    colorsObject.insert(
        QStringLiteral("textColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).foreground().color().name());

    colorsObject.insert(
        QStringLiteral("backgroundColor"),
        KColorScheme(QPalette::Active, KColorScheme::View).background().color().name());

    QVariantHash mapping;
    mapping.insert(QStringLiteral("contactGroup"), contactGroupObject);
    mapping.insert(QStringLiteral("colors"), colorsObject);

    Grantlee::Context context(mapping);

    if (form == SelfcontainedForm) {
        return d->mSelfcontainedTemplate->render(&context);
    } else if (form == EmbeddableForm) {
        return d->mEmbeddableTemplate->render(&context);
    } else {
        return QString();
    }
}
