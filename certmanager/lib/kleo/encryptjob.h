/*  -*- mode: C++; c-file-style: "gnu" -*-
    encryptjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_ENCRYPTJOB_H__
#define __KLEO_ENCRYPTJOB_H__

#include "job.h"

#include <qcstring.h>

#include <vector>

namespace GpgME {
  class Error;
  class Key;
  class EncryptionResult;
}


namespace Kleo {

  /**
     @short An abstract base class for asynchronous encrypters

     To use a EncryptJob, first obtain an instance from the
     CryptoBackend implementation, connect the progress() and result()
     signals to suitable slots and then start the encryption with a
     call to start(). This call might fail, in which case the
     EncryptJob instance will have scheduled it's own destruction with
     a call to QObject::deleteLater().

     After result() is emitted, the EncryptJob will schedule it's own
     destruction by calling QObject::deleteLater().
  */
  class EncryptJob : public Job {
    Q_OBJECT
  protected:
    EncryptJob( QObject * parent, const char * name );
    ~EncryptJob();

  public:
    /**
       Starts the encryption operation. \a recipients is the a list of
       keys to encrypt \a plainText to. Empty (null) keys are ignored.

       If \a alwaysTrust is true, validity checking for the keys will
       not be performed, but full validity assumed for all keys
       without further checks.
    */
    virtual GpgME::Error start( const std::vector<GpgME::Key> & recipients,
				const QByteArray & plainText, bool alwaysTrust=false ) = 0;

  signals:
    void result( const GpgME::EncryptionResult & result, const QByteArray & cipherText );
  };

}

#endif // __KLEO_ENCRYPTJOB_H__
