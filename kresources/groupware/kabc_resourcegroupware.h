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
#ifndef KABC_RESOURCEGROUPWARE_H
#define KABC_RESOURCEGROUPWARE_H

#include <kabcresourcecached.h>

#include <libkdepim/progressmanager.h>

#include <kio/job.h>

class KConfig;

namespace KABC {

class GroupwarePrefs;

class ResourceGroupware : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceGroupware( const KConfig * );
    ResourceGroupware( const KURL &url,
                       const QString &user, const QString &password );
    ~ResourceGroupware();

    void readConfig( const KConfig * );
    void writeConfig( KConfig * );

    void readAddressBooks();
    void writeAddressBooks();

    void retrieveAddressBooks();

    GroupwarePrefs *prefs() const { return mPrefs; }

    bool doOpen();
    void doClose();

    Ticket *requestSaveTicket();
    void releaseSaveTicket( Ticket* );

    bool load();
    bool asyncLoad();
    bool save( Ticket * );
    bool asyncSave( Ticket * );

  protected:
    void init();

  private slots:
    void slotJobResult( KIO::Job * );
    void slotJobData( KIO::Job *, const QByteArray & );
    void slotJobPercent( KIO::Job *job, unsigned long percent );

    void cancelLoad();

  private:
    GroupwarePrefs *mPrefs;

    KIO::TransferJob *mDownloadJob;
    KPIM::ProgressItem *mProgress;
    QString mJobData;
};

}

#endif
