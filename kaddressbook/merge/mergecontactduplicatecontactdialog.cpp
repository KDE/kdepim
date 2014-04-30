/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#include "mergecontactduplicatecontactdialog.h"

#include "mergecontactshowresulttabwidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KGlobal>

using namespace KABMergeContacts;

MergeContactDuplicateContactDialog::MergeContactDuplicateContactDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Contacts to merge" ) );
    setButtons( Close );
    mMergeContact = new MergeContactShowResultTabWidget;
    setMainWidget(mMergeContact);
    readConfig();
}

MergeContactDuplicateContactDialog::~MergeContactDuplicateContactDialog()
{

}

void MergeContactDuplicateContactDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactDuplicateContactDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactDuplicateContactDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void MergeContactDuplicateContactDialog::slotAddDuplicateContact()
{
    //TODO
}
