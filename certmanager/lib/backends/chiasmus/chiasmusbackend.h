/*  -*- mode: C++; c-file-style: "gnu" -*-
    chiasmusbackend.h

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


#ifndef __KLEO_CHIASMUSBACKEND_H__
#define __KLEO_CHIASMUSBACKEND_H__

#include "kleo/cryptobackend.h"

class CryptPlugWrapper;

namespace Kleo {
  class CryptoConfig;
}
class QString;

namespace Kleo {

  class ChiasmusBackend : public Kleo::CryptoBackend {
  public:
    ChiasmusBackend();
    ~ChiasmusBackend();

    QString name() const;
    QString displayName() const;

    Kleo::CryptoConfig * config() const;

    Kleo::CryptoBackend::Protocol * openpgp() const { return 0; }
    Kleo::CryptoBackend::Protocol * smime() const { return 0; }
    Kleo::CryptoBackend::Protocol * protocol( const char * name ) const;

    bool checkForOpenPGP( QString * reason=0 ) const;
    bool checkForSMIME( QString * reason=0 ) const;
    bool checkForChiasmus( QString * reason=0 ) const;
    bool checkForProtocol( const char * name, QString * reason=0 ) const;

    bool supportsOpenPGP() const { return false; }
    bool supportsSMIME() const { return false; }
    bool supportsProtocol( const char * name ) const;

    const char * enumerateProtocols( int i ) const;

  private:
    class CryptoConfig;
    class Protocol;
    mutable CryptoConfig * mCryptoConfig;
    mutable Protocol * mProtocol;
  };

}


#endif // __KLEO_CHIASMUSBACKEND_H__
