/*
 * Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "selectaddressbookdialog.h"

#include <KLocalizedString>
#include <KConfigGroup>

#include <kcontacts/addressee.h>
#include <KSharedConfig>

using namespace KPIM;
class KPIM::SelectAddressBookDialogPrivate
{
public:
    SelectAddressBookDialogPrivate(SelectAddressBookDialog *qq)
        : q(qq)
    {
        const QStringList mimeTypes(KContacts::Addressee::mimeType());
        q->setMimeTypeFilter(mimeTypes);
        q->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
        q->setWindowTitle(i18nc("@title:window", "Select Address Book"));
        q->setDescription(
            i18nc("@info",
                  "Select the address book where the contact will be saved:"));
        q->changeCollectionDialogOptions(Akonadi::CollectionDialog::KeepTreeExpanded);
        readConfig();
    }
    ~SelectAddressBookDialogPrivate()
    {
        writeConfig();
    }

    SelectAddressBookDialog *q;
    void readConfig();
    void writeConfig();
};

void SelectAddressBookDialogPrivate::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SelectAddressBookDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        q->resize(size);
    }
}

void SelectAddressBookDialogPrivate::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SelectAddressBookDialog");
    group.writeEntry("Size", q->size());
    group.sync();
}


SelectAddressBookDialog::SelectAddressBookDialog(QWidget *parent)
    : Akonadi::CollectionDialog(parent),
      d(new KPIM::SelectAddressBookDialogPrivate(this))
{
}

SelectAddressBookDialog::~SelectAddressBookDialog()
{
    delete d;
}

