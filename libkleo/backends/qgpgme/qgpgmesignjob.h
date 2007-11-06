/*
    qgpgmesignjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004, 2007 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_QGPGMESIGNJOB_H__
#define __KLEO_QGPGMESIGNJOB_H__

#include "kleo/signjob.h"

#include "qgpgmejob.h"

#include <gpgme++/signingresult.h>


namespace GpgME {
  class Error;
  class Context;
  class Key;
}

namespace Kleo {

  class QGpgMESignJob : public SignJob, private QGpgMEJob {
    Q_OBJECT QGPGME_JOB
  public:
    explicit QGpgMESignJob( GpgME::Context * context );
    ~QGpgMESignJob();

    /*! \reimp from SignJob */
    GpgME::Error start( const std::vector<GpgME::Key> & signers,
			const QByteArray & plainText,
			GpgME::SignatureMode mode );

    /*! \reimp from SignJob */
    void start( const std::vector<GpgME::Key> & signers,
                const boost::shared_ptr<QIODevice> & plainText,
                const boost::shared_ptr<QIODevice> & signature,
                GpgME::SignatureMode mode );

    /*! \reimp from SignJob */
    GpgME::SigningResult exec( const std::vector<GpgME::Key> & signers,
			       const QByteArray & plainText,
			       GpgME::SignatureMode mode,
			       QByteArray & signature );

    /*! \reimp from Job */
    void showErrorDialog( QWidget * parent, const QString & caption ) const;

  private Q_SLOTS:
    void slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & e ) {
      QGpgMEJob::doSlotOperationDoneEvent( context, e );
    }

  private:
    void doOperationDoneEvent( const GpgME::Error & e );
    GpgME::Error setup( const std::vector<GpgME::Key> &, const QByteArray &, GpgME::SignatureMode );
    void setup( const std::vector<GpgME::Key> &, const boost::shared_ptr<QIODevice> &, const boost::shared_ptr<QIODevice> &, GpgME::SignatureMode );

  private:
    GpgME::SigningResult mResult;
  };

}

#endif // __KLEO_QGPGMESIGNJOB_H__
