/*
    hierarchicalkeylistjob.h

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

#ifndef __KLEO_HIERARCHICALKEYLISTJOB_H__
#define __KLEO_HIERARCHICALKEYLISTJOB_H__

#include <kleo/keylistjob.h>
#include <kleo/cryptobackend.h>
#include <kdepimmacros.h>

#include <gpgmepp/keylistresult.h>

#include <qcstring.h>
#include <qguardedptr.h>

#include <set>

namespace GpgME {
  class Error;
  class Key;
}

namespace Kleo {
  class KeyListJob;
}

namespace Kleo {

  /**
     @short A convenience job that additionally fetches all available issuers.

     To use a HierarchicalKeyListJob, pass it a CryptoBackend
     implementation, connect the progress() and result() signals to
     suitable slots and then start the keylisting with a call to
     start(). This call might fail, in which case the
     HierarchicalKeyListJob instance will have scheduled it's own
     destruction with a call to QObject::deleteLater().

     After result() is emitted, the HierarchicalKeyListJob will
     schedule its own destruction by calling QObject::deleteLater().
  */
  class KDE_EXPORT HierarchicalKeyListJob : public KeyListJob {
    Q_OBJECT
  public:
    HierarchicalKeyListJob( const CryptoBackend::Protocol * protocol,
			    bool remote=false, bool includeSigs=false, bool validating=false );
    ~HierarchicalKeyListJob();

    /**
       Starts the keylist operation. \a patterns is a list of patterns
       used to restrict the list of keys returned. Empty patterns are
       ignored. \a patterns must not be empty or contain only empty
       patterns; use the normal KeyListJob for a full listing.

       The \a secretOnly parameter is ignored by
       HierarchicalKeyListJob and must be set to false.
    */
    GpgME::Error start( const QStringList & patterns, bool secretOnly=false );

    GpgME::KeyListResult exec( const QStringList & patterns, bool secretOnly,
			       std::vector<GpgME::Key> & keys );

  private slots:
    void slotResult( const GpgME::KeyListResult & );
    void slotNextKey( const GpgME::Key & key );
    /*! \reimp from Job */
    void slotCancel();

  private:
    GpgME::Error startAJob();

  private:
    const CryptoBackend::Protocol * const mProtocol;
    const bool mRemote;
    const bool mIncludeSigs;
    const bool mValidating;
    bool mTruncated;
    std::set<QString> mSentSet; // keys already sent (prevent duplicates even if the backend should return them)
    std::set<QString> mScheduledSet; // keys already scheduled (by starting a job for them)
    std::set<QString> mNextSet; // keys to schedule for the next iteraton
    GpgME::KeyListResult mIntermediateResult;
    QGuardedPtr<KeyListJob> mJob;
  };

}

#endif // __KLEO_HIERARCHICALKEYLISTJOB_H__
