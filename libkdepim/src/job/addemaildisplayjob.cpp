/*
  Copyright 2013-2015 Laurent Montel <montel@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "addemaildisplayjob.h"
#include "misc/broadcaststatus.h"
#include "akonadi/contact/selectaddressbookdialog.h"

#include <CollectionDialog>
#include <Akonadi/Contact/ContactSearchJob>
#include <Item>
#include <ItemCreateJob>
#include <ItemModifyJob>
#include <CollectionFetchJob>
#include <CollectionFetchScope>
#include <Collection>
#include <Akonadi/Contact/ContactEditorDialog>
#include <AgentTypeDialog>
#include <AgentType>
#include <AgentFilterProxyModel>
#include <AgentInstanceCreateJob>

#include <KContacts/Addressee>
#include <KContacts/ContactGroup>

#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>

using namespace KPIM;

class Q_DECL_HIDDEN AddEmailDiplayJob::Private
{
public:
    Private(AddEmailDiplayJob *qq, const QString &emailString, QWidget *parentWidget)
        : q(qq),
          mShowAsHTML(false),
          mRemoteContent(false),
          mCompleteAddress(emailString),
          mParentWidget(parentWidget)
    {
        KContacts::Addressee::parseEmailAddress(emailString, mName, mEmail);
    }

    void slotResourceCreationDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }
        createContact();
    }

    void searchContact()
    {
        // first check whether a contact with the same email exists already
        Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob(q);
        searchJob->setLimit(1);
        searchJob->setQuery(Akonadi::ContactSearchJob::Email, mEmail.toLower(),
                            Akonadi::ContactSearchJob::ExactMatch);
        q->connect(searchJob, SIGNAL(result(KJob*)), q, SLOT(slotSearchDone(KJob*)));
    }

    void modifyContact()
    {
        Akonadi::Item item = contact;
        if (item.hasPayload<KContacts::Addressee>()) {
            KContacts::Addressee address = item.payload<KContacts::Addressee>();
            address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailPreferedFormatting"), mShowAsHTML ? QStringLiteral("HTML") : QStringLiteral("TEXT"));
            address.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailAllowToRemoteContent"), mRemoteContent ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));
            item.setPayload<KContacts::Addressee>(address);
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
            q->connect(job, SIGNAL(result(KJob*)), SLOT(slotAddModifyContactDone(KJob*)));
        } else {
            searchContact();
        }
    }

    void slotSearchDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }

        const Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob *>(job);

        const Akonadi::Item::List items = searchJob->items();
        if (items.isEmpty()) {
            createContact();
        } else {
            Akonadi::Item item = items.at(0);
            KContacts::Addressee contact = searchJob->contacts().at(0);
            contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailPreferedFormatting"), mShowAsHTML ? QStringLiteral("HTML") : QStringLiteral("TEXT"));
            contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailAllowToRemoteContent"), mRemoteContent ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));
            item.setPayload<KContacts::Addressee>(contact);
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
            q->connect(job, SIGNAL(result(KJob*)), SLOT(slotAddModifyContactDone(KJob*)));
        }
    }

    void createContact()
    {
        const QStringList mimeTypes(KContacts::Addressee::mimeType());

        Akonadi::CollectionFetchJob *const addressBookJob =
            new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),
                                            Akonadi::CollectionFetchJob::Recursive);

        addressBookJob->fetchScope().setContentMimeTypes(mimeTypes);
        q->connect(addressBookJob, SIGNAL(result(KJob*)), SLOT(slotCollectionsFetched(KJob*)));
    }

    void slotCollectionsFetched(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
            q->emitResult();
            return;
        }

        const Akonadi::CollectionFetchJob *addressBookJob =
            qobject_cast<Akonadi::CollectionFetchJob *>(job);

        Akonadi::Collection::List canCreateItemCollections;

        foreach (const Akonadi::Collection &collection, addressBookJob->collections()) {
            if (Akonadi::Collection::CanCreateItem & collection.rights()) {
                canCreateItemCollections.append(collection);
            }
        }

        Akonadi::Collection addressBook;

        const int nbItemCollection(canCreateItemCollections.size());
        if (nbItemCollection == 0) {
            if (KMessageBox::questionYesNo(
                        mParentWidget,
                        i18nc("@info",
                              "You must create an address book before adding a contact. Do you want to create an address book?"),
                        i18nc("@title:window", "No Address Book Available")) == KMessageBox::Yes) {
                Akonadi::AgentTypeDialog dlg(mParentWidget);
                dlg.setWindowTitle(i18n("Add Address Book"));
                dlg.agentFilterProxyModel()->addMimeTypeFilter(KContacts::Addressee::mimeType());
                dlg.agentFilterProxyModel()->addMimeTypeFilter(KContacts::ContactGroup::mimeType());
                dlg.agentFilterProxyModel()->addCapabilityFilter(QStringLiteral("Resource"));

                if (dlg.exec()) {
                    const Akonadi::AgentType agentType = dlg.agentType();

                    if (agentType.isValid()) {
                        Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob(agentType, q);
                        q->connect(job, SIGNAL(result(KJob*)), SLOT(slotResourceCreationDone(KJob*)));
                        job->configure(mParentWidget);
                        job->start();
                        return;
                    } else { //if agent is not valid => return error and finish job
                        q->setError(UserDefinedError);
                        q->emitResult();
                        return;
                    }
                } else { //Canceled create agent => return error and finish job
                    q->setError(UserDefinedError);
                    q->emitResult();
                    return;
                }
            } else {
                q->setError(UserDefinedError);
                q->emitResult();
                return;
            }
        } else if (nbItemCollection == 1) {
            addressBook = canCreateItemCollections[0];
        } else {
            // ask user in which address book the new contact shall be stored
            QPointer<Akonadi::SelectAddressBookDialog> dlg = new Akonadi::SelectAddressBookDialog(mParentWidget);

            bool gotIt = true;
            if (dlg->exec()) {
                addressBook = dlg->selectedCollection();
            } else {
                q->setError(UserDefinedError);
                q->emitResult();
                gotIt = false;
            }
            delete dlg;
            if (!gotIt) {
                return;
            }
        }

        if (!addressBook.isValid()) {
            q->setError(UserDefinedError);
            q->emitResult();
            return;
        }
        KContacts::Addressee contact;
        contact.setNameFromString(mName);
        contact.insertEmail(mEmail, true);
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailPreferedFormatting"), mShowAsHTML ? QStringLiteral("HTML") : QStringLiteral("TEXT"));
        contact.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("MailAllowToRemoteContent"), mRemoteContent ? QStringLiteral("TRUE") : QStringLiteral("FALSE"));

        // create the new item
        Akonadi::Item item;
        item.setMimeType(KContacts::Addressee::mimeType());
        item.setPayload<KContacts::Addressee>(contact);

        // save the new item in akonadi storage
        Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(item, addressBook, q);
        q->connect(createJob, SIGNAL(result(KJob*)), SLOT(slotAddModifyContactDone(KJob*)));
    }

    void slotAddModifyContactDone(KJob *job)
    {
        if (job->error()) {
            q->setError(job->error());
            q->setErrorText(job->errorText());
        }
        q->emitResult();
    }

    AddEmailDiplayJob *q;
    Akonadi::Item contact;
    bool mShowAsHTML;
    bool mRemoteContent;
    QString mCompleteAddress;
    QString mEmail;
    QString mName;
    QWidget *mParentWidget;
};

AddEmailDiplayJob::AddEmailDiplayJob(const QString &email, QWidget *parentWidget, QObject *parent)
    : KJob(parent), d(new Private(this, email, parentWidget))
{
}

AddEmailDiplayJob::~AddEmailDiplayJob()
{
    delete d;
}

void AddEmailDiplayJob::setShowAsHTML(bool html)
{
    d->mShowAsHTML = html;
}

void AddEmailDiplayJob::setRemoteContent(bool b)
{
    d->mRemoteContent = b;
}

void AddEmailDiplayJob::setContact(const Akonadi::Item &contact)
{
    d->contact = contact;
}

void AddEmailDiplayJob::start()
{
    if (d->contact.isValid()) {
        d->modifyContact();
    } else {
        d->searchContact();
    }
}

#include "moc_addemaildisplayjob.cpp"
