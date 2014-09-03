/*
 * Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
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

#include "selectedcollectiondialog.h"

#include <KLocalizedString>
#include <KConfigGroup>

#include <kabc/addressee.h>
#include <KSharedConfig>

namespace KPIM
{

SelectedCollectionDialog::SelectedCollectionDialog(QWidget *parent)
    : Akonadi::CollectionDialog(parent)
{
    const QStringList mimeTypes(KABC::Addressee::mimeType());
    setMimeTypeFilter(mimeTypes);
    setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    setCaption(i18nc("@title:window", "Select Address Book"));
    setDescription(
        i18nc("@info",
              "Select the address book where the contact will be saved:"));
    changeCollectionDialogOptions(Akonadi::CollectionDialog::KeepTreeExpanded);
    readConfig();
}

SelectedCollectionDialog::~SelectedCollectionDialog()
{
    writeConfig();
}

void SelectedCollectionDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SelectedCollectionDialog");
    const QSize size = group.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void SelectedCollectionDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SelectedCollectionDialog");
    group.writeEntry("Size", size());
    group.sync();
}
}
