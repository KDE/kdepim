/*  -*- mode: C++; c-file-style: "gnu" -*-
    deletejob.h

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

#ifndef __KLEO_DELETEJOB_H__
#define __KLEO_DELETEJOB_H__

#include "job.h"

namespace GpgME {
  class Error;
  class Key;
}

namespace Kleo {

  /**
     @short An abstract base class for asynchronous deleters

     To use a DeleteJob, first obtain an instance from the
     CryptoBackend implementation, connect the progress() and result()
     signals to suitable slots and then start the delete with a call
     to start(). This call might fail, in which case the DeleteJob
     instance will have scheduled it's own destruction with a call to
     QObject::deleteLater().

     After result() is emitted, the DeleteJob will schedule it's own
     destruction by calling QObject::deleteLater().
  */
  class DeleteJob : public Job {
    Q_OBJECT
  protected:
    DeleteJob( QObject * parent, const char * name );
    ~DeleteJob();

  public:
    /**
       Starts the delete operation. \a key represents the key to
       delete, \a allowSecretKeyDeletion specifies if a key may also
       be deleted if the secret key part is available, too.
    */
    virtual GpgME::Error start( const GpgME::Key & key, bool allowSecretKeyDeletion=false ) = 0;

  signals:
    void result( const GpgME::Error & result );
  };

}

#endif // __KLEO_DELETEJOB_H__
