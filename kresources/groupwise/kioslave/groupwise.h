/*
    This file is part of KDE.

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
#ifndef GROUPWISE_H
#define GROUPWISE_H

#include <kio/slavebase.h>

#include <qobject.h>
//Added by qt3to4:
#include <QByteArray>

class Groupwise : public QObject, public KIO::SlaveBase
{
    Q_OBJECT
  public:
    void get( const KUrl &url );
    Groupwise( const QByteArray &protocol, const QByteArray &pool,
      const QByteArray &app );

  protected:
    void debugMessage( const QString & );
    void errorMessage( const QString & );

    void getFreeBusy( const KUrl &url );
    void getCalendar( const KUrl &url );
    void getAddressbook( const KUrl &url );
    void updateAddressbook( const KUrl &url );

    QString soapUrl( const KUrl &url );

  protected slots:
    void slotReadAddressBookTotalSize( int );
    void slotReadAddressBookProcessedSize( int );
    void slotServerErrorMessage( const QString &, bool );
    void slotReadReceiveAddressees( const KABC::Addressee::List );
};

#endif
