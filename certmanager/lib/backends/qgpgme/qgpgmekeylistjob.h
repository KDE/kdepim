/*
    qgpgmekeylistjob.h

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

#ifndef __KLEO_QGPGMEKEYLISTJOB_H__
#define __KLEO_QGPGMEKEYLISTJOB_H__

#include <kleo/keylistjob.h>

#include <gpgmepp/keylistresult.h>

#include "qgpgmejob.h"

namespace GpgME {
  class Error;
  class Context;
  class Key;
}

namespace Kleo {

  class QGpgMEKeyListJob : public KeyListJob, private QGpgMEJob {
    Q_OBJECT QGPGME_JOB
  public:
    QGpgMEKeyListJob( GpgME::Context * context );
    ~QGpgMEKeyListJob();

    /*! \reimp from KeyListJob */
    GpgME::Error start( const QStringList & patterns, bool secretOnly );

    /*! \reimp from KeyListJob */
    GpgME::KeyListResult exec( const QStringList & patterns, bool secretOnly, std::vector<GpgME::Key> & keys );

    /*! \reimp from Job */
    void showErrorDialog( QWidget * parent, const QString & caption ) const;

  private slots:
    void slotNextKeyEvent( GpgME::Context * context, const GpgME::Key & key );
    void slotOperationDoneEvent( GpgME::Context * context, const GpgME::Error & e ) {
      QGpgMEJob::doSlotOperationDoneEvent( context, e );
    }

  private:
    void doOperationDoneEvent( const GpgME::Error & e );
    void setup( const QStringList & );

  private:
    GpgME::KeyListResult mResult;
  };

}

#endif // __KLEO_QGPGMEKEYLISTJOB_H__
