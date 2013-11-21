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


#include "nepomukdebugdialog.h"
#include "utils.h"

#include "pimcommon/nepomukdebug/akonadiresultlistview.h"

#include <Akonadi/Item>

#include <KLocale>

#include <QStringListModel>

NepomukDebugDialog::NepomukDebugDialog(QItemSelectionModel *selectionModel, QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Nepomuk Debug"));
    setButtons(Close);
    setDefaultButton( Close );
    mListView = new PimCommon::AkonadiResultListView;
    QStringListModel *resultModel = new QStringListModel( this );
    mListView->setModel( resultModel );
    setMainWidget( mListView );
    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(selectionModel);
    QStringList uidList;
    Q_FOREACH ( const Akonadi::Item &item, lst ) {
        uidList << QString::number( item.id() );
    }
    resultModel->setStringList( uidList );
    readConfig();
}

NepomukDebugDialog::~NepomukDebugDialog()
{
    writeConfig();
}

void NepomukDebugDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "NepomukDebugDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NepomukDebugDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "NepomukDebugDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}


#include "moc_nepomukdebugdialog.cpp"
