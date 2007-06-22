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
#ifndef SLOXACCOUNTS_H
#define SLOXACCOUNTS_H

#include <kabc/addressee.h>
#include <QObject>
#include "slox_export.h"

class KJob;
namespace KIO {
class Job;
}

class SloxBase;

class KSLOX_EXPORT SloxAccounts : public QObject
{
    Q_OBJECT
  public:
    SloxAccounts( SloxBase *res, const KUrl &baseUrl );
    ~SloxAccounts();

    void insertUser( const QString &id, const KABC::Addressee &a );

    KABC::Addressee lookupUser( const QString &id );

    QString lookupId( const QString &email );

  protected:
    void requestAccounts();
    void readAccounts();

    QString cacheFile() const;

  protected slots:
    void slotResult( KJob * );

  private:
    QString mDomain;

    KIO::Job *mDownloadJob;

    QMap<QString, KABC::Addressee> mUsers; // map users ids to addressees.

    KUrl mBaseUrl;
    SloxBase *mRes;
};

#endif
