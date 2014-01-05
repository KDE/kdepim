/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "adblockblockableitemsdialog.h"
#include "adblockblockableitemswidget.h"

#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <KMenu>

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QWebFrame>

using namespace MessageViewer;
AdBlockBlockableItemsDialog::AdBlockBlockableItemsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Blockable Items") );
    setButtons( Ok|Cancel );

    mBlockableItems = new AdBlockBlockableItemsWidget;

    setMainWidget(mBlockableItems);
    readConfig();
}

AdBlockBlockableItemsDialog::~AdBlockBlockableItemsDialog()
{
    writeConfig();
}

void AdBlockBlockableItemsDialog::setWebFrame(QWebFrame *frame)
{
    mBlockableItems->setWebFrame(frame);
}

void AdBlockBlockableItemsDialog::saveFilters()
{
    mBlockableItems->saveFilters();
}

void AdBlockBlockableItemsDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockBlockableItemsDialog" );
    group.writeEntry( "Size", size() );
}

void AdBlockBlockableItemsDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AdBlockBlockableItemsDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(500,300) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}


