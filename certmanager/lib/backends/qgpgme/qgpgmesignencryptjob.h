/*
    qgpgmesignencryptjob.h

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

#ifndef __KLEO_QGPGMESIGNENCRYPTJOB_H__
#define __KLEO_QGPGMESIGNENCRYPTJOB_H__

#include <kleo/signencryptjob.h>

#include "qgpgmejob.h"

#include <gpgmepp/signingresult.h>
#include <gpgmepp/encryptionresult.h>

#include <qcstring.h>

#include <utility>

#include <kdepimmacros.h>
namespace GpgME {
  class Error;
  class Context;
  class Key;
}

namespace Kleo {

  class KDE_EXPORT QGpgMESignEncryptJob : public SignEncryptJob, private QGpgMEJob {
    Q_OBJECT QGPGME_JOB
  public:
    QGpgMESignEncryptJob( GpgME::Context * context );
    ~QGpgMESignEncryptJob();

    /*! \reimp from SignEncryptJob */
    GpgME::Error start( const std::vector<GpgME::Key> & signers,
			const std::vector<GpgME::Key> & recipients,
			const QByteArray & plainText, bool alwaysTrust );

    std::pair<GpgME::SigningResult,GpgME::EncryptionResult>
      exec( const std::vector<GpgME::Key> & signers,
	    const std::vector<GpgME::Key> & recipients,
	    const QByteArray & plainText, bool alwaysTrust,
	    QByteArray & cipherText );

    /*! \reimp from Job */
    void showErrorDialog( QWidget * parent, const QString & caption ) const;

  private slots:
    void slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & e ) {
      QGpgMEJob::doSlotOperationDoneEvent( context, e );
    }

  private:
    void doOperationDoneEvent( const GpgME::Error & e );
    GpgME::Error setup( const std::vector<GpgME::Key> &,
			const QByteArray & );
  private:
    std::pair<GpgME::SigningResult,GpgME::EncryptionResult> mResult;
  };

}

#endif // __KLEO_QGPGMESIGNENCRYPTJOB_H__
