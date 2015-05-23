/*
  Copyright (c) 2010 Volker Krause <vkrause@kde.org>

  Based on kmail/kmlineeditspell.h/cpp
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

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

#include "composerlineedit.h"

#include "addressline/recentaddress/recentaddresses.h"
#include "addressline/completionconfiguredialog/completionconfiguredialog.h"
#include "settings/messagecomposersettings.h"
#include "messageviewer/utils/autoqpointer.h"

#include <messagecore/utils/stringutil.h>

#include <KEmailAddress>
#include <kcontacts/vcarddrag.h>
#include <kcontacts/contactgroup.h>
#include <kcontacts/vcardconverter.h>

#include <kio/netaccess.h>
#include <qmenu.h>
#include <qurl.h>
#include <kmessagebox.h>
#include <kcompletionbox.h>
#include <KLocalizedString>

#include <QFile>
#include <QCursor>
#include <QKeyEvent>
#include <QDropEvent>
#include <kcontacts/contactgrouptool.h>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <QtCore/QBuffer>

using namespace MessageComposer;

ComposerLineEdit::ComposerLineEdit(bool useCompletion, QWidget *parent)
    : KPIM::AddresseeLineEdit(parent, useCompletion),
      m_recentAddressConfig(MessageComposerSettings::self()->config())
      , mAutoGroupExpand(false)
      , mExpandIntern(true)
{
    allowSemicolonAsSeparator(MessageComposerSettings::allowSemicolonAsAddressSeparator());
    loadContacts();
    setEnableBalooSearch(MessageComposerSettings::showBalooSearchInComposer());
    connect(this, &ComposerLineEdit::editingFinished, this, &ComposerLineEdit::slotEditingFinished);
    connect(this, &ComposerLineEdit::textCompleted, this, &ComposerLineEdit::slotEditingFinished);

    KConfigGroup group(KSharedConfig::openConfig(), "AddressLineEdit");
    mAutoGroupExpand = group.readEntry("AutoGroupExpand", false);
}

//-----------------------------------------------------------------------------
void ComposerLineEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) &&
            !completionBox()->isVisible()) {
        Q_EMIT focusDown();
        AddresseeLineEdit::keyPressEvent(e);
        return;
    } else if (e->key() == Qt::Key_Up) {
        Q_EMIT focusUp();
        return;
    } else if (e->key() == Qt::Key_Down) {
        Q_EMIT focusDown();
        return;
    }
    AddresseeLineEdit::keyPressEvent(e);
}

void ComposerLineEdit::insertEmails(const QStringList &emails)
{
    if (emails.empty()) {
        return;
    }

    QString contents = text();
    if (!contents.isEmpty()) {
        contents += QLatin1Char(',');
    }
    // only one address, don't need kpopup to choose
    if (emails.size() == 1) {
        setText(contents + emails.front());
        return;
    }
    //multiple emails, let the user choose one
    QMenu menu(this);
    menu.setTitle(i18n("Select email from contact"));
    menu.setObjectName(QLatin1String("Addresschooser"));
    for (QStringList::const_iterator it = emails.constBegin(), end = emails.constEnd() ; it != end; ++it) {
        menu.addAction(*it);
    }
    const QAction *result = menu.exec(QCursor::pos());
    if (!result) {
        return;
    }
    setText(contents + KLocalizedString::removeAcceleratorMarker(result->text()));
}

#ifndef QT_NO_DRAGANDDROP
void ComposerLineEdit::dropEvent(QDropEvent *event)
{
    const QMimeData *md = event->mimeData();

    // Case one: The user dropped a text/directory (i.e. vcard), so decode its
    //           contents
    if (KContacts::VCardDrag::canDecode(md)) {
        KContacts::Addressee::List list;
        KContacts::VCardDrag::fromMimeData(md, list);

        KContacts::Addressee::List::ConstIterator ait;
        KContacts::Addressee::List::ConstIterator end(list.constEnd());
        for (ait = list.constBegin(); ait != end; ++ait) {
            insertEmails((*ait).emails());
        }
    }

    // Case two: The user dropped a list or Urls.
    // Iterate over that list. For mailto: Urls, just add the addressee to the list,
    // and for other Urls, download the Url and assume it points to a vCard
    else if (md->hasUrls()) {
        QList<QUrl> urls = md->urls();
        KContacts::Addressee::List list;

        foreach (const QUrl &url, urls) {

            // First, let's deal with mailto Urls. The path() part contains the
            // email-address.
            if (url.scheme() == QLatin1String("mailto")) {
                KContacts::Addressee addressee;
                addressee.insertEmail(KEmailAddress::decodeMailtoUrl(url), true /* preferred */);
                list += addressee;
            }

            // Otherwise, download the vCard to which the Url points
            else {
                KContacts::VCardConverter converter;
                QString fileName;
                if (KIO::NetAccess::download(url, fileName, parentWidget())) {
                    QFile file(fileName);
                    if (file.open(QIODevice::ReadOnly)) {
                        QByteArray data = file.readAll();
                        file.close();
                        list += converter.parseVCards(data);
                        KIO::NetAccess::removeTempFile(fileName);

                        if (list.isEmpty()) {  // try to parse a contact group
                            KContacts::ContactGroup group;
                            QBuffer dataStream(&data);
                            dataStream.open(QIODevice::ReadOnly);
                            QString error;
                            if (KContacts::ContactGroupTool::convertFromXml(&dataStream, group, &error)) {
                                Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob(group);
                                connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &ComposerLineEdit::groupExpandResult);
                                expandJob->start();
                            }
                        }
                    }
                } else {
                    const QString caption(i18n("vCard Import Failed"));
                    const QString text = i18n("<qt>Unable to access <b>%1</b>.</qt>", url.url());
                    KMessageBox::error(parentWidget(), text, caption);
                }
            }
        }

        // Now, let the user choose which addressee to add.
        foreach (const KContacts::Addressee &addressee, list) {
            insertEmails(addressee.emails());
        }
    }

    // Case three: Let AddresseeLineEdit deal with the rest
    else {
        KPIM::AddresseeLineEdit::dropEvent(event);
    }
}
#endif

void ComposerLineEdit::groupExpandResult(KJob *job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);

    if (!expandJob) {
        return;
    }

    const KContacts::Addressee::List contacts = expandJob->contacts();
    foreach (const KContacts::Addressee &addressee, contacts) {
        if (mExpandIntern || text().isEmpty()) {
            insertEmails(QStringList() << addressee.fullEmail());
        } else {
            Q_EMIT addAddress(addressee.fullEmail());
        }
    }

    job->deleteLater();
}

void ComposerLineEdit::slotEditingFinished()
{
    foreach (KJob *job, mMightBeGroupJobs) {
        disconnect(job);
        job->deleteLater();
    }

    mMightBeGroupJobs.clear();
    mGroups.clear();

    if (!text().isEmpty()) {
        QStringList addresses = KEmailAddress::splitAddressList(text());
        foreach (const QString &address, addresses) {
            Akonadi::ContactGroupSearchJob *job = new Akonadi::ContactGroupSearchJob();
            job->setQuery(Akonadi::ContactGroupSearchJob::Name, address);
            connect(job, &Akonadi::ContactGroupSearchJob::result, this, &ComposerLineEdit::slotGroupSearchResult);
            mMightBeGroupJobs.append(job);
        }
    }
}

void ComposerLineEdit::slotGroupSearchResult(KJob *job)
{
    Akonadi::ContactGroupSearchJob *searchJob = qobject_cast<Akonadi::ContactGroupSearchJob *>(job);

    Q_ASSERT(mMightBeGroupJobs.contains(searchJob));
    mMightBeGroupJobs.removeOne(searchJob);

    const KContacts::ContactGroup::List contactGroups = searchJob->contactGroups();
    if (contactGroups.isEmpty()) {
        return; // Nothing todo, probably a normal email address was entered
    }

    mGroups.append(contactGroups);
    searchJob->deleteLater();

    if (mAutoGroupExpand) {
        expandGroups();
    }
}

void ComposerLineEdit::expandGroups()
{
    QStringList addresses = KEmailAddress::splitAddressList(text());

    foreach (const KContacts::ContactGroup &group, mGroups) {
        Akonadi::ContactGroupExpandJob *expandJob = new Akonadi::ContactGroupExpandJob(group);
        connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &ComposerLineEdit::groupExpandResult);
        addresses.removeAll(group.name());
        expandJob->start();
    }
    setText(addresses.join(QStringLiteral(", ")));
    mGroups.clear();
}

void ComposerLineEdit::slotToggleExpandGroups()
{
    mAutoGroupExpand = !mAutoGroupExpand;
    KConfigGroup group(KSharedConfig::openConfig(), "AddressLineEdit");
    group.writeEntry("AutoGroupExpand", mAutoGroupExpand);
}

#ifndef QT_NO_CONTEXTMENU
void ComposerLineEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<QMenu> popup = createStandardContextMenu();
    if (popup) {
        popup->exec(e->globalPos());
        delete popup;
    }
}
#endif

void ComposerLineEdit::configureCompletionOrder(QMenu *menu)
{
    if (menu) {   // can be 0 on platforms with only a touch interface
        if (isCompletionEnabled()) {
            menu->addSeparator();
            QAction *act = menu->addAction(i18n("Configure Completion..."));
            connect(act, &QAction::triggered, this, &ComposerLineEdit::configureCompletion);
        }
        menu->addSeparator();
        QAction *act = menu->addAction(i18n("Automatically expand groups"));
        act->setCheckable(true);
        act->setChecked(mAutoGroupExpand);
        connect(act, &QAction::triggered, this, &ComposerLineEdit::slotToggleExpandGroups);

        if (!mGroups.isEmpty()) {
            act = menu->addAction(i18n("Expand Groups..."));
            connect(act, &QAction::triggered, this, &ComposerLineEdit::expandGroups);
        }
    }
}

void ComposerLineEdit::configureCompletion()
{
    MessageViewer::AutoQPointer<KPIM::CompletionConfigureDialog> dlg(new KPIM::CompletionConfigureDialog(this));
    dlg->setRecentAddresses(KPIM::RecentAddresses::self(m_recentAddressConfig)->addresses());
    dlg->setLdapClientSearch(ldapSearch());
    dlg->setEmailBlackList(balooBlackList());
    dlg->load();
    if (dlg->exec() && dlg) {
        if (dlg->recentAddressWasChanged()) {
            KPIM::RecentAddresses::self(m_recentAddressConfig)->clear();
            dlg->storeAddresses(m_recentAddressConfig);
            loadContacts();
            updateBalooBlackList();
            updateCompletionOrder();
        }
    }
}

//-----------------------------------------------------------------------------
void ComposerLineEdit::loadContacts()
{
    const QString recentAddressGroupName = i18n("Recent Addresses");
    if (MessageComposerSettings::self()->showRecentAddressesInComposer()) {
        const QStringList recent =
            cleanupEmailList(KPIM::RecentAddresses::self(m_recentAddressConfig)->addresses());
        QStringList::ConstIterator it = recent.constBegin();
        QString name, email;

        KSharedConfig::Ptr config = KSharedConfig::openConfig(QLatin1String("kpimcompletionorder"));
        KConfigGroup group(config, "CompletionWeights");
        const int weight = group.readEntry("Recent Addresses", 10);
        removeCompletionSource(recentAddressGroupName);
        const int idx = addCompletionSource(recentAddressGroupName, weight);

        QStringList::ConstIterator end = recent.constEnd();
        for (; it != end; ++it) {
            KContacts::Addressee addr;
            KEmailAddress::extractEmailAddressAndName(*it, email, name);
            name = KEmailAddress::quoteNameIfNecessary(name);
            if ((name[0] == QLatin1Char('"')) && (name[name.length() - 1] == QLatin1Char('"'))) {
                name.remove(0, 1);
                name.truncate(name.length() - 1);
            }
            addr.setNameFromString(name);
            addr.insertEmail(email, true);
            addContact(addr, weight, idx);
        }
    } else {
        removeCompletionSource(recentAddressGroupName);
    }
}

void ComposerLineEdit::setRecentAddressConfig(KConfig *config)
{
    m_recentAddressConfig = config;
}

bool ComposerLineEdit::expandIntern() const
{
    return mExpandIntern;
}

void ComposerLineEdit::setExpandIntern(bool expand)
{
    mExpandIntern = expand;
}
