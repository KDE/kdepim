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
#include "stockageauthviewdialog.h"
#include "stockageauthviewwidget.h"

#include <KLocale>

using namespace PimCommon;

StockageAuthViewDialog::StockageAuthViewDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Authorize" ) );
    setButtons( User1 |Ok | Cancel );

    mView = new StockageAuthViewWidget;
    setMainWidget(mView);
    readConfig();
    connect(this, SIGNAL(user1Clicked()), SIGNAL(getToken()));
}

StockageAuthViewDialog::~StockageAuthViewDialog()
{
    writeConfig();
}

void StockageAuthViewDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "StockageAuthViewDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StockageAuthViewDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "StockageAuthViewDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void StockageAuthViewDialog::setUrl(const QUrl &url)
{
    mView->setUrl(url);
}

