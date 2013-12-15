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

#include "mergecontactsdialog.h"
#include "utils.h"

#include <Akonadi/Item>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

#include <QItemSelectionModel>
#include <QLabel>

MergeContactsDialog::MergeContactsDialog(QItemSelectionModel *selectionModel, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Ok | Cancel );
    readConfig();

    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(selectionModel);
    if (lst.count() < 2) {
        enableButtonOk(false);
        setMainWidget(new QLabel(i18n("You must select at least two elements.")));
    } else {
        setMainWidget(new QLabel(i18n("You select %1", lst.count())));
    }
}

MergeContactsDialog::~MergeContactsDialog()
{
    writeConfig();
}

void MergeContactsDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactsDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactsDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

#include "moc_mergecontactsdialog.cpp"
