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

#include "baloodebugdialog.h"
#include "baloodebugwidget.h"

#include <QVBoxLayout>
#include <KSharedConfig>


using namespace PimCommon;

BalooDebugDialog::BalooDebugDialog(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Close);

    //Don't translate it's just a dialog to debug
    setCaption(QLatin1String("Debug baloo"));

    mBalooDebugWidget = new BalooDebugWidget(this);
    mBalooDebugWidget->setObjectName(QLatin1String("baloodebugwidget"));
    setMainWidget(mBalooDebugWidget);
    readConfig();
}

BalooDebugDialog::~BalooDebugDialog()
{
    writeConfig();
}

void BalooDebugDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "BalooDebugDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void BalooDebugDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "BalooDebugDialog" );
    group.writeEntry( "Size", size() );
}


void BalooDebugDialog::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    mBalooDebugWidget->setAkonadiId(akonadiId);
}

void BalooDebugDialog::setSearchType(BalooDebugSearchPathComboBox::SearchType type)
{
    mBalooDebugWidget->setSearchType(type);
}

void BalooDebugDialog::doSearch()
{
    mBalooDebugWidget->doSearch();
}


