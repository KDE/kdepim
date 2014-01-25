/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicepropertiesdialog.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QLabel>

StorageServicePropertiesDialog::StorageServicePropertiesDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Properties"));

    setButtons( Close );
    mInformation = new QLabel;
    setMainWidget(mInformation);
    readConfig();
}

StorageServicePropertiesDialog::~StorageServicePropertiesDialog()
{
    writeConfig();
}

void StorageServicePropertiesDialog::setInformation(const QString &info)
{
    mInformation->setText(info);
}

void StorageServicePropertiesDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "StorageServicePropertiesDialog" );
    const QSize size = group.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void StorageServicePropertiesDialog::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("StorageServicePropertiesDialog") );
    group.writeEntry( "Size", size() );
}
