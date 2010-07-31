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
#include <kdepimmacros.h>
#include <tqobject.h>

namespace KIO {
class Job;
}

class SloxBase;

class KDE_EXPORT SloxAccounts : public QObject
{
    Q_OBJECT
  public:
    SloxAccounts( SloxBase *res, const KURL &baseUrl );
    ~SloxAccounts();

    void insertUser( const TQString &id, const KABC::Addressee &a );

    KABC::Addressee lookupUser( const TQString &id );

    TQString lookupId( const TQString &email );

  protected:
    void requestAccounts();
    void readAccounts();

    TQString cacheFile() const;

  protected slots:
    void slotResult( KIO::Job * );

  private:
    TQString mDomain;

    KIO::Job *mDownloadJob;

    TQMap<TQString, KABC::Addressee> mUsers; // map users ids to addressees.

    KURL mBaseUrl;
    SloxBase *mRes;
};

#endif
