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

#include "searchduplicatecontactwizard.h"
#include "potentialduplicatecontactswidget.h"
#include "contactselectionwidget.h"

#include <KABC/Addressee>

#include <KLocale>
#include <KConfigGroup>

SearchDuplicateContactWizard::SearchDuplicateContactWizard(QItemSelectionModel *selectionModel, QWidget *parent)
    : KAssistantDialog(parent)
{
    mSelectionWidget = new ContactSelectionWidget( selectionModel, this );
    mSelectionWidget->setMessageText( i18n( "Search potential duplicate contacts" ) );

    mSelectionPageItem = new KPageWidgetItem( mSelectionWidget, i18n( "Select Contacts" ) );
    addPage( mSelectionPageItem );
    setAppropriate( mSelectionPageItem, true );

    mPotentialDuplicateContactsWidget = new PotentialDuplicateContactsWidget(this);
    mDuplicateContactsPageItem = new KPageWidgetItem( mSelectionWidget, i18n( "Potential Duplicate Contacts" ) );
    addPage( mDuplicateContactsPageItem );
    setAppropriate( mDuplicateContactsPageItem, true );

    readConfig();
}

SearchDuplicateContactWizard::~SearchDuplicateContactWizard()
{
    writeConfig();
}

void SearchDuplicateContactWizard::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "SearchDuplicateContactWizard" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void SearchDuplicateContactWizard::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "SearchDuplicateContactWizard");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void SearchDuplicateContactWizard::accept()
{
    mergeContacts();
    close();
    setResult(QDialog::Accepted);
}

void SearchDuplicateContactWizard::next()
{
    if (currentPage() == mSelectionPageItem) {
        Akonadi::Item::List list = mSelectionWidget->selectedContactsItem();
        mPotentialDuplicateContactsWidget->setAddressList(list);
    }
    KAssistantDialog::next();
}

void SearchDuplicateContactWizard::mergeContacts()
{
    //TODO
}

#include "moc_searchduplicatecontactwizard.cpp"
