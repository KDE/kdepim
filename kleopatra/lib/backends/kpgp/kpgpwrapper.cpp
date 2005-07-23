/*  -*- mode: C++; c-file-style: "gnu" -*-
    kpgpwrapper.cpp

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kpgpwrapper.h"

#include "kpgpbackendbase.h"

#include <kpgpbase.h>

#include <backends/kpgp/kpgpkeylistjob.h>
//#include <backends/kpgp/kpgpencryptjob.h>
//#include <backends/kpgp/kpgpdecryptjob.h>
//#include <backends/kpgp/kpgpsignjob.h>
//#include <backends/kpgp/kpgpverifydetachedjob.h>
//#include <backends/kpgp/kpgpverifyopaquejob.h>
//#include <backends/kpgp/kpgpkeygenerationjob.h>
//#include <backends/kpgp/kpgpimportjob.h>
//#include <backends/kpgp/kpgpexportjob.h>
//#include <backends/kpgp/kpgpsecretkeyexportjob.h>
//#include <backends/kpgp/kpgpdownloadjob.h>
//#include <backends/kpgp/kpgpdeletejob.h>
//#include <backends/kpgp/kpgpsignencryptjob.h>
//#include <backends/kpgp/kpgpdecryptverifyjob.h>
//#include <backends/kpgp/kpgpcryptoconfig.h>

KpgpWrapper::KpgpWrapper( const QString & name )
  : mName( name ),
    mPgpBase( 0 )
{

}

KpgpWrapper::~KpgpWrapper()
{

}

QString KpgpWrapper::name() const
{
  return mName;
}

QString KpgpWrapper::displayName() const
{
  return mName;
}

Kleo::KeyListJob * KpgpWrapper::keyListJob( bool /*remote*/,
                                            bool /*includeSigs*/,
                                            bool /*validate*/ ) const
{
  return new Kleo::KpgpKeyListJob( pgpBase() );
}

Kleo::EncryptJob * KpgpWrapper::encryptJob( bool /*armor*/,
                                            bool /*textmode*/ ) const
{
  return 0;
}

Kleo::DecryptJob * KpgpWrapper::decryptJob() const
{
  return 0;
}

Kleo::SignJob * KpgpWrapper::signJob( bool /*armor*/, bool /*textMode*/ ) const
{
  return 0;
}

Kleo::VerifyDetachedJob * KpgpWrapper::verifyDetachedJob( bool /*textmode*/ ) const
{
  return 0;
}

Kleo::VerifyOpaqueJob * KpgpWrapper::verifyOpaqueJob( bool /*textmode*/ ) const
{
  return 0;
}

Kleo::KeyGenerationJob * KpgpWrapper::keyGenerationJob() const
{
  return 0;
}

Kleo::ImportJob * KpgpWrapper::importJob() const
{
  return 0;
}

Kleo::ExportJob * KpgpWrapper::publicKeyExportJob( bool /*armor*/ ) const
{
  return 0;
}

Kleo::ExportJob * KpgpWrapper::secretKeyExportJob( bool /*armor*/ ) const
{
  return 0;
}

Kleo::DownloadJob * KpgpWrapper::downloadJob( bool /*armor*/ ) const
{
  return 0;
}

Kleo::DeleteJob * KpgpWrapper::deleteJob() const
{
  return 0;
}

Kleo::SignEncryptJob * KpgpWrapper::signEncryptJob( bool /*armor*/,
                                                    bool /*textMode*/ ) const
{
  return 0;
}

Kleo::DecryptVerifyJob * KpgpWrapper::decryptVerifyJob( bool /*textmode*/ ) const
{
  return 0;
}

Kleo::RefreshKeysJob * KpgpWrapper::refreshKeysJob() const
{
  return 0;
}

Kpgp::Base * KpgpWrapper::pgpBase() const
{
  if ( !mPgpBase ) {
    if ( name() == GPG1_BACKEND_NAME )
      mPgpBase = new Kpgp::BaseG();
    else if ( name() == PGP2_BACKEND_NAME )
      mPgpBase = new Kpgp::Base2();
    else if ( name() == PGP5_BACKEND_NAME )
      mPgpBase = new Kpgp::Base5();
    else if ( name() == PGP6_BACKEND_NAME )
      mPgpBase = new Kpgp::Base6();
  }
  return mPgpBase;
}
