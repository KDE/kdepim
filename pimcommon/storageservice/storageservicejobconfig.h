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

#ifndef STORAGESERVICEJOBCONFIG_H
#define STORAGESERVICEJOBCONFIG_H

#include <QObject>
#include "pimcommon/pimcommon_export.h"

namespace PimCommon {
class ISettingsJob;
class PIMCOMMON_EXPORT StorageServiceJobConfig : public QObject
{
    Q_OBJECT
public:
    ~StorageServiceJobConfig();
    static StorageServiceJobConfig *self();

    void registerConfigIf(ISettingsJob *settingsJob);

    QString youSendItApiKey() const;
    QString dropboxOauthConsumerKey() const;
    QString dropboxOauthSignature() const;
    QString defaultUploadFolder() const;
    QString boxClientId() const;
    QString boxClientSecret() const;
    QString hubicClientId() const;
    QString hubicClientSecret() const;    
    QString dropboxRootPath() const;
    QString oauth2RedirectUrl() const;
    QString hubicScope() const;
    QString gdriveClientId() const;
    QString gdriveClientSecret() const;


private:
    StorageServiceJobConfig(QObject *parent=0);
    friend class StorageServiceJobConfigPrivate;
    ISettingsJob *mSettingsJob;
};
}
#endif // STORAGESERVICEJOBCONFIG_H
