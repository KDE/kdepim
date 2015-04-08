/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "xxportmanager.h"
#include "contactselectiondialog.h"

#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionDialog>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/Item>
#include <AkonadiCore/ItemCreateJob>

#include <KLocalizedString>
#include <KMessageBox>
#include <QProgressDialog>

#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QAction>
#include <QItemSelectionModel>
#include <QWidget>

XXPortManager::XXPortManager(QWidget *parent)
    : QObject(parent), mSelectionModel(Q_NULLPTR),
      mParentWidget(parent), mImportProgressDialog(Q_NULLPTR)
{
    mImportMapper = new QSignalMapper(this);
    mExportMapper = new QSignalMapper(this);

    connect(mImportMapper, static_cast<void (QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &XXPortManager::slotImport);
    connect(mExportMapper, static_cast<void (QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &XXPortManager::slotExport);
}

XXPortManager::~XXPortManager()
{
}

void XXPortManager::addImportAction(QAction *action, const QString &identifier)
{
    mImportMapper->setMapping(action, identifier);
    connect(action, SIGNAL(triggered(bool)), mImportMapper, SLOT(map()));
}

void XXPortManager::addExportAction(QAction *action, const QString &identifier)
{
    mExportMapper->setMapping(action, identifier);
    connect(action, SIGNAL(triggered(bool)), mExportMapper, SLOT(map()));
}

void XXPortManager::setSelectionModel(QItemSelectionModel *selectionModel)
{
    mSelectionModel = selectionModel;
}

void XXPortManager::setDefaultAddressBook(const Akonadi::Collection &addressBook)
{
    mDefaultAddressBook = addressBook;
}

void XXPortManager::importFile(const QUrl &url)
{
    QString identifier;
    if (url.path().endsWith(QLatin1String("vcf"))) {
        identifier = QLatin1String("vcard30");
    } else if (url.path().endsWith(QLatin1String("ldif"))) {
        identifier = QLatin1String("ldif");
    } else if (url.path().endsWith(QLatin1String("gmx"))) {
        identifier = QLatin1String("gmx");
    }
    if (identifier.isEmpty()) {
        return;
    }
    XXPort *xxport = mFactory.createXXPort(identifier, mParentWidget);
    if (!xxport) {
        return;
    }
    xxport->setOption(QLatin1String("importUrl"), url.path());
    ContactList contactList = xxport->importContacts();

    delete xxport;
    import(contactList);
}

void XXPortManager::slotImport(const QString &identifier)
{
    const XXPort *xxport = mFactory.createXXPort(identifier, mParentWidget);
    if (!xxport) {
        return;
    }
    ContactList contactList = xxport->importContacts();

    delete xxport;
    import(contactList);
}

void XXPortManager::import(const ContactList &contacts)
{
    if (contacts.isEmpty()) {   // nothing to import
        return;
    }

    const QStringList mimeTypes(KContacts::Addressee::mimeType());

    QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog(mParentWidget);
    dlg->setMimeTypeFilter(mimeTypes);
    dlg->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    dlg->setCaption(i18n("Select Address Book"));
    dlg->setDescription(
        i18n("Select the address book the imported contact(s) shall be saved in:"));
    dlg->setDefaultCollection(mDefaultAddressBook);

    if (!dlg->exec() || !dlg) {
        delete dlg;
        return;
    }

    const Akonadi::Collection collection = dlg->selectedCollection();
    delete dlg;

    if (!mImportProgressDialog) {
        mImportProgressDialog = new QProgressDialog(mParentWidget);
        mImportProgressDialog->setWindowTitle(i18n("Import Contacts"));
        mImportProgressDialog->setLabelText(
            i18np("Importing one contact to %2", "Importing %1 contacts to %2",
                  contacts.count(), collection.name()));
        mImportProgressDialog->setCancelButton(Q_NULLPTR);
        mImportProgressDialog->setAutoClose(true);
        mImportProgressDialog->setRange(1, contacts.count());
    }

    mImportProgressDialog->show();

    for (int i = 0; i < contacts.addressList().count(); ++i) {
        Akonadi::Item item;
        item.setPayload<KContacts::Addressee>(contacts.addressList().at(i));
        item.setMimeType(KContacts::Addressee::mimeType());

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob(item, collection);
        connect(job, &Akonadi::ItemCreateJob::result, this, &XXPortManager::slotImportJobDone);
    }
    for (int i = 0; i < contacts.contactGroupList().count(); ++i) {
        Akonadi::Item groupItem(KContacts::ContactGroup::mimeType());
        groupItem.setPayload<KContacts::ContactGroup>(contacts.contactGroupList().at(i));

        Akonadi::Job *createJob = new Akonadi::ItemCreateJob(groupItem, collection);
        connect(createJob, &KJob::result, this, &XXPortManager::slotImportJobDone);
    }

}

void XXPortManager::slotImportJobDone(KJob *)
{
    if (!mImportProgressDialog) {
        return;
    }

    mImportProgressDialog->setValue(mImportProgressDialog->value() + 1);

    // cleanup on last step
    if (mImportProgressDialog->value() == mImportProgressDialog->maximum()) {
        mImportProgressDialog->deleteLater();
        mImportProgressDialog = Q_NULLPTR;
    }
}

void XXPortManager::slotExport(const QString &identifier)
{
    if (!mSelectionModel) {
        return;
    }

    const bool selectExportType = (identifier == QLatin1String("vcard21") || identifier == QLatin1String("vcard30") || identifier == QLatin1String("vcard40"));
    QPointer<ContactSelectionDialog> dlg =
        new ContactSelectionDialog(mSelectionModel, selectExportType, mParentWidget);
    dlg->setMessageText(i18n("Which contact do you want to export?"));
    dlg->setDefaultAddressBook(mDefaultAddressBook);
    if (!dlg->exec() || !dlg) {
        delete dlg;
        return;
    }

    const KContacts::AddresseeList contacts = dlg->selectedContacts().addressList();
    const VCardExportSelectionWidget::ExportFields exportFields = dlg->exportType();
    delete dlg;

    if (contacts.isEmpty()) {
        KMessageBox::sorry(Q_NULLPTR, i18n("You have not selected any contacts to export."));
        return;
    }

    const XXPort *xxport = mFactory.createXXPort(identifier, mParentWidget);
    if (!xxport) {
        return;
    }
    ContactList contactLists;
    contactLists.setAddressList(contacts);
    xxport->exportContacts(contactLists, exportFields);

    delete xxport;
}

