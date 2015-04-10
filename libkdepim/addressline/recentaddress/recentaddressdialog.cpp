/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "recentaddressdialog.h"
#include "recentaddresswidget.h"
#include "recentaddresses.h"
#include <kpimutils/email.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KLocale>

#include <QCoreApplication>
#include <QLayout>
#include <QVBoxLayout>

using namespace KPIM;

RecentAddressDialog::RecentAddressDialog( QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n( "Edit Recent Addresses" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    mRecentAddressWidget = new RecentAddressWidget(this);
    setMainWidget( mRecentAddressWidget );
    readConfig();
}

RecentAddressDialog::~RecentAddressDialog()
{
    writeConfig();
}

void RecentAddressDialog::setAddresses( const QStringList &addrs )
{
    mRecentAddressWidget->setAddresses(addrs);
}

QStringList RecentAddressDialog::addresses() const
{
    return mRecentAddressWidget->addresses();
}

void RecentAddressDialog::storeAddresses(KConfig *config)
{
    mRecentAddressWidget->storeAddresses(config);
}

bool RecentAddressDialog::wasChanged() const
{
    return mRecentAddressWidget->wasChanged();
}

void RecentAddressDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void RecentAddressDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "RecentAddressDialog" );
    group.writeEntry( "Size", size() );
    group.sync();
}
