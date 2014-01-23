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

#ifndef STORAGESERVICEINTERFACE_H
#define STORAGESERVICEINTERFACE_H

#include <QString>

namespace PimCommon {

class ISettingsJob
{
public:
    virtual ~ISettingsJob() {}
    //YouSendit
    virtual QString youSendItApiKey() const = 0;

    //DropBox
    virtual QString dropboxOauthConsumerKey() const = 0;
    virtual QString dropboxOauthSignature() const = 0;
    virtual QString dropboxRootPath() const = 0;

    //Box
    virtual QString boxClientId() const = 0;
    virtual QString boxClientSecret() const = 0;

    //Hubic
    virtual QString hubicClientId() const = 0;
    virtual QString hubicClientSecret() const = 0;
    virtual QString hubicScope() const = 0;

    virtual QString oauth2RedirectUrl() const = 0;

    //UbuntuOne
    virtual QString ubuntuOneAttachmentVolume() const = 0;
    virtual QString ubuntuOneTokenName() const = 0;

    //GDrive
    virtual QString gdriveClientId() const = 0;
    virtual QString gdriveClientSecret() const = 0;


    virtual QString defaultUploadFolder() const = 0;
};

}

#endif // STORAGESERVICEINTERFACE_H
