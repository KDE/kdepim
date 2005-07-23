/*  -*- mode: C++; c-file-style: "gnu" -*-
    kpgpbackendbase.cpp

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kpgpbackendbase.h"

#include "pgp2backend.h"
#include "pgp5backend.h"
#include "pgp6backend.h"
#include "gpg1backend.h"

#include <klocale.h>

#include <qstring.h>

#include "kpgpwrapper.h"

Kleo::KpgpBackendBase::KpgpBackendBase()
  : Kleo::CryptoBackend(),
    mOpenPGPProtocol( 0 )
{
}

Kleo::KpgpBackendBase::~KpgpBackendBase()
{
  delete mOpenPGPProtocol; mOpenPGPProtocol = 0;
}

QString Kleo::GPG1Backend::name() const {
  return GPG1_BACKEND_NAME;
}

QString Kleo::GPG1Backend::displayName() const {
  return i18n("Kpgp/gpg");
}

QString Kleo::PGP2Backend::name() const {
  return PGP2_BACKEND_NAME;
}

QString Kleo::PGP2Backend::displayName() const {
  return i18n("Kpgp/pgp v2");
}

QString Kleo::PGP5Backend::name() const {
  return PGP5_BACKEND_NAME;
}

QString Kleo::PGP5Backend::displayName() const {
  return i18n("Kpgp/pgp v5");
}

QString Kleo::PGP6Backend::name() const {
  return PGP6_BACKEND_NAME;
}

QString Kleo::PGP6Backend::displayName() const {
  return i18n("Kpgp/pgp v6");
}

static const QString notSupported() {
  return i18n("This backend does not support S/MIME");
}

bool Kleo::KpgpBackendBase::checkForOpenPGP( QString * /*reason*/ ) const {
  return true;
}

bool Kleo::KpgpBackendBase::checkForSMIME( QString * reason ) const {
  if ( reason ) *reason = notSupported();
  return false;
}

Kleo::CryptoBackend::Protocol * Kleo::KpgpBackendBase::openpgp() const {
  if ( !mOpenPGPProtocol )
    if ( checkForOpenPGP() )
      mOpenPGPProtocol = new KpgpWrapper( name() );
  return mOpenPGPProtocol;
}
