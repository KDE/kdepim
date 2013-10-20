/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "selectiontypedialog.h"
#include "selectiontypetreewidget.h"

#include <KLocale>
#include <QHBoxLayout>

SelectionTypeDialog::SelectionTypeDialog(QWidget *parent)
    :KDialog(parent)
{
    setCaption( i18n( "Select Type" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mSelectionTreeWidget = new SelectionTypeTreeWidget(this);
    mainLayout->addWidget(mSelectionTreeWidget);
    setMainWidget(mainWidget);
    readConfig();
}

SelectionTypeDialog::~SelectionTypeDialog()
{
    writeConfig();
}

void SelectionTypeDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SelectionTypeDialog" );
    group.writeEntry( "Size", size() );
}

void SelectionTypeDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SelectionTypeDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

Utils::StoredTypes SelectionTypeDialog::kmailTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->kmailStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::kaddressbookTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->kaddressbookStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::kalarmTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->kalarmStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::korganizerTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->korganizerStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::kjotsTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->kjotsStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::knotesTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->knotesStoredType(numberOfStep);
}

Utils::StoredTypes SelectionTypeDialog::akregatorTypesSelected(int &numberOfStep) const
{
    return mSelectionTreeWidget->akregatorStoredType(numberOfStep);
}

#include "selectiontypedialog.moc"
