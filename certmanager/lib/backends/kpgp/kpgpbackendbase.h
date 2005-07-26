/*  -*- mode: C++; c-file-style: "gnu" -*-
    kpgpbackendbase.h

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


#ifndef __KLEO_KPGPBACKENDBASE_H__
#define __KLEO_KPGPBACKENDBASE_H__

#include "kleo/cryptobackend.h"

#define GPG1_BACKEND_NAME "Kpgp/gpg1"
#define PGP2_BACKEND_NAME "Kpgp/pgp2"
#define PGP5_BACKEND_NAME "Kpgp/pgp5"
#define PGP6_BACKEND_NAME "Kpgp/pgp6"

namespace Kleo {
  class CryptoConfig;
}
class QString;
class KpgpWrapper;

namespace Kleo {

  class KpgpBackendBase : public Kleo::CryptoBackend {
  public:
    KpgpBackendBase();
    ~KpgpBackendBase();

    CryptoConfig * config() const { return 0; }
    Protocol * openpgp() const;
    Protocol * smime() const { return 0; }

    bool supportsOpenPGP() const { return true; }
    bool supportsSMIME() const { return false; }

    bool checkForOpenPGP( QString * reason=0 ) const;
    bool checkForSMIME( QString * reason=0 ) const;
  private:
    mutable KpgpWrapper * mOpenPGPProtocol;
  };

}


#endif // __KLEO_KPGPBACKENDBASE_H__
