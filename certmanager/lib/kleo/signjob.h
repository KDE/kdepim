/*  -*- mode: C++; c-file-style: "gnu" -*-
    signjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klar�lvdalens Datakonsult AB

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

#ifndef __KLEO_SIGNJOB_H__
#define __KLEO_SIGNJOB_H__

#include <gpgmepp/context.h> // for Context::SignatureMode (or should
			     // we roll our own enum here?)
#include "job.h"
#include <qcstring.h>

#include <vector>

namespace GpgME {
  class Error;
  class Key;
  class SigningResult;
}


namespace Kleo {

  /**
     @short An abstract base class for asynchronous signing

     To use a SignJob, first obtain an instance from the CryptoBackend
     implementation, connect the progress() and result() signals to
     suitable slots and then start the signing with a call to
     start(). This call might fail, in which case the SignJob instance
     will have scheduled it's own destruction with a call to
     QObject::deleteLater().

     After result() is emitted, the SignJob will schedule it's own
     destruction by calling QObject::deleteLater().
  */
  class SignJob : public Job {
    Q_OBJECT
  protected:
    SignJob( QObject * parent, const char * name );
    ~SignJob();

  public:
    /**
       Starts the signing operation. \a signers is the list of keys to
       sign \a plainText with. Empty (null) keys are ignored.
    */
    virtual GpgME::Error start( const std::vector<GpgME::Key> & signers,
				const QByteArray & plainText,
				GpgME::Context::SignatureMode mode ) = 0;

  signals:
    void result( const GpgME::SigningResult & result, const QByteArray & signature );
  };

}

#endif // __KLEO_SIGNJOB_H__
