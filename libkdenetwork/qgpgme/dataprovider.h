/* dataprovider.h
   Copyright (C) 2004 Klarälvdalens Datakonsult AB

   This file is part of QGPGME.
 
   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with QGPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

// -*- c++ -*-
#ifndef __QGPGME_DATAPROVIDER_H__
#define __QGPGME_DATAPROVIDER_H__

#include <gpgmepp/interfaces/dataprovider.h>

#include <qcstring.h>
#include <kdepimmacros.h>

namespace QGpgME {

  class KDE_EXPORT QByteArrayDataProvider : public GpgME::DataProvider {
  public:
    QByteArrayDataProvider();
    QByteArrayDataProvider( const QByteArray & initialData );
    ~QByteArrayDataProvider();

    const QByteArray & data() const { return mArray; }

  private:
    // these shall only be accessed through the dataprovider
    // interface, where they're public:
    /*! \reimp */
    bool isSupported( Operation ) const { return true; }
    /*! \reimp */
    ssize_t read( void * buffer, size_t bufSize );
    /*! \reimp */
    ssize_t write( const void * buffer, size_t bufSize );
    /*! \reimp */
    off_t seek( off_t offset, int whence );
    /*! \reimp */
    void release();

  private:
    QByteArray mArray;
    off_t mOff;
  };

} // namespace QGpgME

#endif // __QGPGME_EVENTLOOPINTERACTOR_H__


