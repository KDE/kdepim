/*
    multideletejob.h

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

#ifndef __KLEO_MULTIDELETEJOB_H__
#define __KLEO_MULTIDELETEJOB_H__

#include <kleo/job.h>
#include <kleo/cryptobackend.h>

#include <kdepimmacros.h>
#include <qguardedptr.h>

#include <vector>

namespace GpgME {
  class Error;
  class Key;
}

namespace Kleo {
  class DeleteJob;
}

namespace Kleo {

  /**
     @short A convenience class bundling together multiple DeleteJobs.

     To use a MultiDeleteJob, pass it a CryptoBackend implementation,
     connect the progress() and result() signals to suitable slots and
     then start the delete with a call to start(). This call might
     fail, in which case the MultiDeleteJob instance will have scheduled
     it's own destruction with a call to QObject::deleteLater().

     After result() is emitted, the MultiDeleteJob will schedule it's own
     destruction by calling QObject::deleteLater().
  */
  class KDE_EXPORT MultiDeleteJob : public Job {
    Q_OBJECT
  public:
    MultiDeleteJob( const CryptoBackend::Protocol * protocol );
    ~MultiDeleteJob();

    /**
       Starts the delete operation. \a keys is the list of keys to
       delete, \a allowSecretKeyDeletion specifies if a key may also
       be deleted if the secret key part is available, too.
    */
    GpgME::Error start( const std::vector<GpgME::Key> & keys, bool allowSecretKeyDeletion=false );

  signals:
    void result( const GpgME::Error & result, const GpgME::Key & errorKey );

  private slots:
    void slotResult( const GpgME::Error & );
    /*! \reimp from Job */
    void slotCancel();

  private:
    GpgME::Error startAJob();

  private:
    const CryptoBackend::Protocol * mProtocol;
    QGuardedPtr<DeleteJob> mJob;
    std::vector<GpgME::Key> mKeys;
    std::vector<GpgME::Key>::const_iterator mIt;
    bool mAllowSecretKeyDeletion;
  };

}

#endif // __KLEO_MULTIDELETEJOB_H__
