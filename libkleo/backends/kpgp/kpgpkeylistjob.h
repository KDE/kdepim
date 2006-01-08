/*  -*- mode: C++ -*-
    kpgpkeylistjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Ingo Kloecker <kloecker@kde.org>

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

#ifndef __KLEO_KPGPKEYLISTJOB_H__
#define __KLEO_KPGPKEYLISTJOB_H__

#include <kleo/keylistjob.h>

#include <qstringlist.h>

namespace GpgME {
  class Error;
  class Context;
  class Key;
}

namespace Kpgp {
  class Base;
}

namespace Kleo {

  class KpgpKeyListJob : public KeyListJob {
    Q_OBJECT
  public:
    KpgpKeyListJob( Kpgp::Base * pgpBase );
    ~KpgpKeyListJob();

    /*! \reimp from KeyListJob */
    GpgME::Error start( const QStringList & patterns, bool secretOnly );

    /*! \reimp from KeyListJob */
    GpgME::KeyListResult exec( const QStringList & patterns, bool secretOnly,
                               std::vector<GpgME::Key> & keys );

    /*! \reimp from Job */
    void slotCancel() { /*FIXME*/ }

  private slots:
    void slotDoIt();

  private:
    Kpgp::Base * mPgpBase;
    QStringList mPatterns;
    bool mSecretOnly;
  };

}

#endif // __KLEO_KPGPKEYLISTJOB_H__
