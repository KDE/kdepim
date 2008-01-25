/*
    This file is part of KDE.

    Copyright (C) 2007 Trolltech ASA. All rights reserved.

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

#ifndef SCALIX_H
#define SCALIX_H

#include <kio/job.h>
#include <kio/slavebase.h>

#include <qobject.h>

class Scalix : public QObject, public KIO::SlaveBase
{
  Q_OBJECT

  public:
    Scalix( const QCString &protocol, const QCString &pool, const QCString &app );

    void get( const KURL &url );
    void put( const KURL &url, int permissions, bool overwrite, bool resume );

  private slots:
    void slotRetrieveResult( KIO::Job* );
    void slotPublishResult( KIO::Job* );
    void slotInfoMessage( KIO::Job*, const QString& );

  private:
    void retrieveFreeBusy( const KURL& );
    void publishFreeBusy( const KURL& );

    QString mFreeBusyData;
};

#endif
