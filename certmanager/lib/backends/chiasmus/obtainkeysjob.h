/*  -*- mode: C++; c-file-style: "gnu" -*-
    obtainkeysjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

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


#ifndef __KLEO_OBTAINKEYSJOB_H__
#define __KLEO_OBTAINKEYSJOB_H__

#include "kleo/specialjob.h"

#include <qstringlist.h>

#include <gpgmepp/context.h>

namespace Kleo {

  /**
     @short SpecialJob for listing Chiasmus key files.

     In the Chiasmus system, keys are mapped 1:1 to
     files. Furthermore, we have to definition of the format of those
     keys, so we cannot display more than the filename anyway. Due to
     all of these limitations, we don't use KeyListJob here, but roll
     our own interface.

     The name of the function is x-obtain-keys. It takes no parameters.

     To use, create an ObtainKeysJob instance like this:
     <code>
     Kleo::SpecialJob * job =
        protocol->specialJob("x-obtain-keys", QMap<QString,QVariant>());
     </code>

     The resulting QVariant will contain a QStringList containing the
     absolute filenames of the keys found in the configured key files.
  */
  class ObtainKeysJob : public Kleo::SpecialJob {
    Q_OBJECT
  public:
    ObtainKeysJob( const QStringList & keypaths );
    ~ObtainKeysJob();

    /*!\reimp SpecialJob */
    GpgME::Error start();
    /*!\reimp SpecialJob */
    GpgME::Error exec( QVariant * result );

    /*!\reimp Kleo::Job */
    void showErrorDialog( QWidget *, const QString & ) const;

  public slots:
    void slotCancel();

  private slots:
    void slotPerform();
    void slotPerform( bool async );

  private:
    GpgME::Error mError;
    const QStringList mKeyPaths;
    unsigned int mIndex;
    QStringList mResult;
    bool mCanceled;
  };

}


#endif // __KLEO_OBTAINKEYSJOB_H__
