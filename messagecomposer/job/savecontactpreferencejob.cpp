/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "savecontactpreferencejob.h"

#include <QInputDialog>
#include <KLocalizedString>

#include <Akonadi/Contact/ContactSearchJob>
#include <AkonadiWidgets/CollectionDialog>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/ItemModifyJob>

#include <QPointer>
#include "messagecomposer_debug.h"

using namespace MessageComposer;

SaveContactPreferenceJob::SaveContactPreferenceJob(const QString &email, const Kleo::KeyResolver::ContactPreferences &pref, QObject *parent)
    : QObject(parent),
      mEmail(email),
      mPref(pref)
{
}

SaveContactPreferenceJob::~SaveContactPreferenceJob()
{

}

void SaveContactPreferenceJob::start()
{
    Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob(this);
    connect(job, &Akonadi::ContactSearchJob::result, this, &SaveContactPreferenceJob::slotSearchContact);
    job->setLimit(1);
    job->setQuery(Akonadi::ContactSearchJob::Email, mEmail);
    job->start();
}

void SaveContactPreferenceJob::slotSearchContact(KJob *job)
{
    Akonadi::ContactSearchJob *contactSearchJob = qobject_cast<Akonadi::ContactSearchJob *>(job);

    const Akonadi::Item::List items = contactSearchJob->items();

    if (items.isEmpty()) {
        bool ok = true;
        const QString fullName = QInputDialog::getText(Q_NULLPTR, i18n("Name Selection"), i18n("Which name shall the contact '%1' have in your address book?", mEmail), QLineEdit::Normal, QString(), &ok);
        if (!ok) {
            deleteLater();
            return;
        }

        QPointer<Akonadi::CollectionDialog> dlg =
            new Akonadi::CollectionDialog(Akonadi::CollectionDialog::KeepTreeExpanded);
        dlg->setMimeTypeFilter(QStringList() << KContacts::Addressee::mimeType());
        dlg->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
        dlg->setDescription(i18n("Select the address book folder to store the new contact in:"));
        if (!dlg->exec()) {
            delete dlg;
            deleteLater();
            return;
        }

        const Akonadi::Collection targetCollection = dlg->selectedCollection();
        delete dlg;

        KContacts::Addressee contact;
        contact.setNameFromString(fullName);
        contact.insertEmail(mEmail, true);
        writeCustomContactProperties(contact, mPref);

        Akonadi::Item item(KContacts::Addressee::mimeType());
        item.setPayload<KContacts::Addressee>(contact);

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, targetCollection);
        connect(job, &Akonadi::ContactSearchJob::result, this, &SaveContactPreferenceJob::slotModifyCreateItem);
    } else {
        Akonadi::Item item = items.first();

        KContacts::Addressee contact = item.payload<KContacts::Addressee>();
        writeCustomContactProperties(contact, mPref);

        item.setPayload<KContacts::Addressee>(contact);

        Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
        connect(job, &Akonadi::ContactSearchJob::result, this, &SaveContactPreferenceJob::slotModifyCreateItem);
    }
}

void SaveContactPreferenceJob::writeCustomContactProperties(KContacts::Addressee &contact, const Kleo::KeyResolver::ContactPreferences &pref) const
{
    contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOENCRYPTPREF"), QLatin1String(Kleo::encryptionPreferenceToString(pref.encryptionPreference)));
    contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOSIGNPREF"), QLatin1String(Kleo::signingPreferenceToString(pref.signingPreference)));
    contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("CRYPTOPROTOPREF"), QLatin1String(cryptoMessageFormatToString(pref.cryptoMessageFormat)));
    contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("OPENPGPFP"), pref.pgpKeyFingerprints.join(QStringLiteral(",")));
    contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("SMIMEFP"), pref.smimeCertFingerprints.join(QStringLiteral(",")));
}

void SaveContactPreferenceJob::slotModifyCreateItem(KJob *job)
{
    if (job->error()) {
        qCDebug(MESSAGECOMPOSER_LOG) << "modify item failed " << job->errorString();
    }
    deleteLater();
}
