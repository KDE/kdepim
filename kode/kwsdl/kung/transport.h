/*
    This file is part of Kung.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <tqobject.h>
#include <kio/job.h>

class Transport : public QObject
{
  Q_OBJECT

  public:
    Transport( const TQString &url );
    void query( const TQString &xml );
  
  signals:
    void result( const TQString &xml );
    void error( const TQString &errorMsg );
  
  private slots:
    void slotData( KIO::Job*, const TQByteArray &data );
    void slotResult( KIO::Job* job );
  
  private:
    TQString mUrl;
    TQByteArray mData;
};

#endif
