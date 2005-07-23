/*  -*- mode: C++; c-file-style: "gnu" -*-
    kpgpwrapper.h

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

#ifndef __KLEO_KPGPWRAPPER_H__
#define __KLEO_KPGPWRAPPER_H__

#include <kleo/cryptobackend.h>

#include <qstring.h>

namespace Kleo {
  class KeyListJob;
  class EncryptJob;
  class DecryptJob;
  class SignJob;
  class VerifyDetachedJob;
  class VerifyOpaqueJob;
  class KeyGenerationJob;
  class ImportJob;
  class ExportJob;
  class DownloadJob;
  class DeleteJob;
  class SignEncryptJob;
  class DecryptVerifyJob;
  class CryptoConfig;
  class RefreshKeysJob;
}

namespace Kpgp {
  class Base;
}

class KpgpWrapper : public Kleo::CryptoBackend::Protocol {
public:
  KpgpWrapper( const QString & name );
  ~KpgpWrapper();

  QString name() const;

  QString displayName() const;

  Kleo::KeyListJob * keyListJob( bool remote=false, bool includeSigs=false,
                                 bool validate=false ) const;
  Kleo::EncryptJob * encryptJob( bool armor=false, bool textmode=false ) const;
  Kleo::DecryptJob * decryptJob() const;
  Kleo::SignJob * signJob( bool armor=false, bool textMode=false ) const;
  Kleo::VerifyDetachedJob * verifyDetachedJob( bool textmode=false) const;
  Kleo::VerifyOpaqueJob * verifyOpaqueJob( bool textmode=false ) const;
  Kleo::KeyGenerationJob * keyGenerationJob() const;
  Kleo::ImportJob * importJob() const;
  Kleo::ExportJob * publicKeyExportJob( bool armor=false ) const;
  Kleo::ExportJob * secretKeyExportJob( bool armor=false ) const;
  Kleo::DownloadJob * downloadJob( bool armor=false ) const;
  Kleo::DeleteJob * deleteJob() const;
  Kleo::SignEncryptJob * signEncryptJob( bool armor=false,
                                         bool textMode=false ) const;
  Kleo::DecryptVerifyJob * decryptVerifyJob( bool textmode=false ) const;
  Kleo::RefreshKeysJob * refreshKeysJob() const;

private:
  Kpgp::Base * pgpBase() const;

private:
  QString mName;
  mutable Kpgp::Base * mPgpBase;
};

#endif // __KLEO_KPGPWRAPPER_H__
