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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef KABC_RESOURCESLOX_H
#define KABC_RESOURCESLOX_H

#include "webdavhandler.h"

#include <kabc/resource.h>
#include <kdepimmacros.h>

#include <qmap.h>
#include <qdom.h>

namespace KIO {
class DavJob;
class Job;
}

namespace KPIM {
class ProgressItem;
}

class KConfig;

namespace KABC {

class SloxPrefs;

class KDE_EXPORT ResourceSlox : public Resource
{
    Q_OBJECT
  public:
    ResourceSlox( const KConfig * );
    ResourceSlox( const KURL &url,
                  const QString &user, const QString &password );
    ~ResourceSlox();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    SloxPrefs *prefs() const { return mPrefs; }

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );

    void insertAddressee( const Addressee &addr );
    void removeAddressee( const Addressee& addr );

    void setReadOnly( bool );
    bool readOnly() const;

  protected:
    void init();

    void parseContactAttribute( const QDomElement &e, Addressee &a );

  protected slots:
    void slotResult( KIO::Job *job );
    void slotProgress( KIO::Job *job, unsigned long percent );

    void cancelDownload();

  private:
    SloxPrefs *mPrefs;

    KIO::DavJob *mDownloadJob;
    KPIM::ProgressItem *mProgress;

    WebdavHandler mWebdavHandler;
};

}

#endif
