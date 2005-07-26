/*
    qgpgmesecretkeyexportjob.h

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

#ifndef __KLEO_QGPGMESECRETKEYEXPORTJOB_H__
#define __KLEO_QGPGMESECRETKEYEXPORTJOB_H__

#include <kleo/exportjob.h>

#include <qcstring.h>

namespace Kleo {
  class GnuPGProcessBase;
}

namespace GpgME {
  class Error;
  class Data;
}

namespace QGpgME {
  class QByteArrayDataProvider;
}

class KProcess;

namespace Kleo {

  class QGpgMESecretKeyExportJob : public ExportJob {
    Q_OBJECT
  public:
    QGpgMESecretKeyExportJob( bool armour );
    ~QGpgMESecretKeyExportJob();

    /*! \reimp from ExportJob */
    GpgME::Error start( const QStringList & patterns );

  private slots:
    /*! \reimp from Job */
    void slotCancel();

    void slotStatus( Kleo::GnuPGProcessBase *, const QString &, const QStringList & );
    void slotStdout( KProcess *, char *, int );
    void slotStderr( KProcess *, char *, int );
    void slotProcessExited( KProcess * );

  private:
    GnuPGProcessBase * mProcess;
    QByteArray mKeyData;
    int mError;
    bool mArmour;
  };

}

#endif // __KLEO_QGPGMESECRETKEYEXPORTJOB_H__
