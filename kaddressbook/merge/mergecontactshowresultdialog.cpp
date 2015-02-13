/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "mergecontactshowresultdialog.h"
#include "widgets/mergecontactshowresulttabwidget.h"
#include "manualmerge/mergecontactinfowidget.h"

#include <KABC/Addressee>

#include <KLocalizedString>
#include <KSharedConfig>

#include <QTabBar>

using namespace KABMergeContacts;

MergeContactShowResultDialog::MergeContactShowResultDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Merged Contact" ) );
    setButtons( Close );
    readConfig();
    mTabWidget = new MergeContactShowResultTabWidget(this);
    mTabWidget->setObjectName(QLatin1String("tabwidget"));
    setMainWidget(mTabWidget);
    updateTabWidget();
}

MergeContactShowResultDialog::~MergeContactShowResultDialog()
{
    writeConfig();
}

void MergeContactShowResultDialog::updateTabWidget()
{
    mTabWidget->updateTabWidget();
}

void MergeContactShowResultDialog::setContacts(const Akonadi::Item::List &lstItem)
{
    mTabWidget->showMergedContacts(lstItem);
}

void MergeContactShowResultDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactShowResultDialog" );
    const QSize size = grp.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void MergeContactShowResultDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "MergeContactShowResultDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}
