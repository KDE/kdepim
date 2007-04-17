/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KABC_RESOURCESLOX_H
#define KABC_RESOURCESLOX_H

#include "sloxbase.h"
#include "webdavhandler.h"

#include <kabc/resourcecached.h>
#include <kdemacros.h>
#include <kabc/addressee.h>

#include <QMap>
#include <qdom.h>
class KJob;
namespace KIO {
class DavJob;
class Job;
}

namespace KPIM {
class ProgressItem;
}

namespace KABC {

class SloxPrefs;

class KDE_EXPORT ResourceSlox : public ResourceCached, public SloxBase
{
    Q_OBJECT
  public:
    ResourceSlox();
    ResourceSlox( const KConfigGroup &group );
    ResourceSlox( const KUrl &url,
                  const QString &user, const QString &password );
    ~ResourceSlox();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    SloxPrefs *prefs() const { return mPrefs; }

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );

    void setReadOnly( bool );
    bool readOnly() const;

  protected:
    void init();

    KABC::PhoneNumber::Type phoneNumberType( const QString &fieldName ) const;
    void parseContactAttribute( const QDomElement &e, Addressee &a );

    void createAddresseeFields( QDomDocument &doc, QDomElement &prop, const Addressee &a );
    void createAddressFields( QDomDocument &doc, QDomElement &parent,
                              const QString &prefix, const KABC::Address &addr );

    void uploadContacts();

  protected slots:
    void slotResult( KJob *job );
    void slotUploadResult( KJob *job );
    void slotProgress( KJob *job, unsigned long percent );

    void cancelDownload();
    void cancelUpload();

  private:
    SloxPrefs *mPrefs;

    KIO::DavJob *mDownloadJob;
    KIO::DavJob *mUploadJob;
    KPIM::ProgressItem *mDownloadProgress;
    KPIM::ProgressItem *mUploadProgress;

    WebdavHandler mWebdavHandler;

    KABC::Addressee mUploadAddressee;

    QMap<KABC::PhoneNumber::Type, QStringList> mPhoneNumberSloxMap, mPhoneNumberOxMap;
};

}

#endif
