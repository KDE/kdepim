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
#ifndef SLOXACCOUNTS_H
#define SLOXACCOUNTS_H

#include <kabc/addressee.h>

#include <qobject.h>

namespace KIO {
class Job;
}

class SloxAccounts : public QObject
{
    Q_OBJECT
  public:
    SloxAccounts( const KURL &baseUrl );
    ~SloxAccounts();

    void insertUser( const QString &id, const KABC::Addressee &a );
  
    KABC::Addressee lookupUser( const QString &id );
  
    QString lookupId( const QString &email );

  protected:
    void requestAccounts();
    void readAccounts();

    QString cacheFile() const;
    
  protected slots:
    void slotResult( KIO::Job * );
  
  private:
    QString mDomain;

    KIO::Job *mDownloadJob;

    QMap<QString, KABC::Addressee> mUsers; // map users ids to addressees.

    KURL mBaseUrl;
};

#endif
