/*
    verifyopaquejob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO_VERIFYOPAQUEJOB_H__
#define __KLEO_VERIFYOPAQUEJOB_H__

#include "job.h"

#include <QtCore/QByteArray>

namespace GpgME {
  class Error;
  class VerificationResult;
}


namespace Kleo {

  /**
     @short An abstract base class for asynchronous verification of opaque signatures

     To use a VerifyOpaqueJob, first obtain an instance from the
     CryptoBackend implementation, connect the progress() and result()
     signals to suitable slots and then start the verification with a
     call to start(). This call might fail, in which case the
     VerifyOpaqueJob instance will have scheduled it's own
     destruction with a call to QObject::deleteLater().

     After result() is emitted, the VerifyOpaqueJob will schedule
     it's own destruction by calling QObject::deleteLater().
  */
  class VerifyOpaqueJob : public Job {
    Q_OBJECT
  protected:
    VerifyOpaqueJob( QObject * parent, const char * name );
  public:
    ~VerifyOpaqueJob();

    /**
       Starts the verification operation. \a signature contains the
       signature data, while \a signedData contains the data over
       which the signature was made.
    */
    virtual GpgME::Error start( const QByteArray & signedData ) = 0;

    /** Synchronous version of @ref start */
    virtual GpgME::VerificationResult exec( const QByteArray & signedData, QByteArray & plainText ) = 0;

  Q_SIGNALS:
    void result( const GpgME::VerificationResult & result, const QByteArray & plainText );
  };

}

#endif // __KLEO_VERIFYOPAQUEJOB_H__
