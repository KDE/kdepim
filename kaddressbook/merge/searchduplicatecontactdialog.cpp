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

#include "searchduplicatecontactdialog.h"

#include <KLocale>
#include <KConfigGroup>

SearchDuplicateContactDialog::SearchDuplicateContactDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Search potential duplicate contacts" ) );
    setButtons( Ok | Cancel );
    readConfig();
}

SearchDuplicateContactDialog::~SearchDuplicateContactDialog()
{
    writeConfig();
}

void SearchDuplicateContactDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "SearchDuplicateContactDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void SearchDuplicateContactDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "SearchDuplicateContactDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

#include "moc_searchduplicatecontactdialog.cpp"
