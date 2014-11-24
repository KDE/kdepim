/* -*- mode: C++; c-file-style: "gnu" -*-
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

#include "addressline/recentaddresses.h"
#include "settings/messagecomposersettings.h"
#include "messageviewer/utils/autoqpointer.h"

#include <messagecore/utils/stringutil.h>

#include <KPIMUtils/kpimutils/email.h>
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
#include <QtCore/QBuffer>
#include <KLocalizedString>
#include <KLocalizedString>

using namespace MessageComposer;

ComposerLineEdit::ComposerLineEdit(bool useCompletion, QWidget *parent)
    : KPIM::AddresseeLineEdit(parent, useCompletion),
      m_recentAddressConfig(MessageComposerSettings::self()->config())
{
    allowSemicolonAsSeparator(MessageComposerSettings::allowSemicolonAsAddressSeparator());
    loadContacts();
}

//-----------------------------------------------------------------------------
void ComposerLineEdit::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) &&
            !completionBox()->isVisible()) {
        emit focusDown();
        AddresseeLineEdit::keyPressEvent(e);
        return;
    } else if (e->key() == Qt::Key_Up) {
        emit focusUp();
        return;
    } else if (e->key() == Qt::Key_Down) {
        emit focusDown();
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
                addressee.insertEmail(KPIMUtils::decodeMailtoUrl(url), true /* preferred */);
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
                                connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &ComposerLineEdit::groupDropExpandResult);
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

void ComposerLineEdit::groupDropExpandResult(KJob *job)
{
    Akonadi::ContactGroupExpandJob *expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);

    if (!expandJob) {
        return;
    }

    const KContacts::Addressee::List contacts = expandJob->contacts();
    foreach (const KContacts::Addressee &addressee, contacts) {
        insertEmails(addressee.emails());
    }

    job->deleteLater();
}

#ifndef QT_NO_CONTEXTMENU
void ComposerLineEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<QMenu> popup = createStandardContextMenu();
    if (popup) {   // can be 0 on platforms with only a touch interface
        popup->addSeparator();
        QAction *act = popup->addAction(i18n("Edit Recent Addresses..."));
        connect(act, &QAction::triggered, this, &ComposerLineEdit::editRecentAddresses);
        popup->exec(e->globalPos());
        delete popup;
    }
}
#endif

void ComposerLineEdit::editRecentAddresses()
{
    MessageViewer::AutoQPointer<KPIM::RecentAddressDialog> dlg(new KPIM::RecentAddressDialog(this));
    dlg->setAddresses(KPIM::RecentAddresses::self(m_recentAddressConfig)->addresses());
    if (dlg->exec() && dlg) {
        KPIM::RecentAddresses::self(m_recentAddressConfig)->clear();
        dlg->addAddresses(m_recentAddressConfig);
        loadContacts();
    }
}

//-----------------------------------------------------------------------------
void ComposerLineEdit::loadContacts()
{
    //AddresseeLineEdit::loadContacts();

    const QString recentAddressGroupName = i18n("Recent Addresses");
    if (MessageComposerSettings::self()->showRecentAddressesInComposer()) {
        const QStringList recent =
            KPIM::RecentAddresses::self(m_recentAddressConfig)->addresses();
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
            KPIMUtils::extractEmailAddressAndName(*it, email, name);
            name = KPIMUtils::quoteNameIfNecessary(name);
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

