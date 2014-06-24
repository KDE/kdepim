/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "storageauthviewdialog.h"
#include "storageauthviewwidget.h"

#include <KLocalizedString>
#include <KSharedConfig>
using namespace PimCommon;

StorageAuthViewDialog::StorageAuthViewDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Authorize" ) );
    setButtons( Ok | Cancel );

    mView = new StorageAuthViewWidget;
    connect(mView, SIGNAL(urlChanged(QUrl)), SIGNAL(urlChanged(QUrl)));
    setMainWidget(mView);
    readConfig();
}

StorageAuthViewDialog::~StorageAuthViewDialog()
{
    writeConfig();
}

void StorageAuthViewDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageAuthViewDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageAuthViewDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "StorageAuthViewDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void StorageAuthViewDialog::setUrl(const QUrl &url)
{
    mView->setUrl(url);
}

