/*
    kleo/enum.cpp

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

#include "enum.h"

#include <klocale.h>

#include <qstring.h>

const char * Kleo::cryptoMessageFormatToString( Kleo::CryptoMessageFormat f ) {
  switch ( f ) {
  default:
  case AutoFormat:
    return "auto";
  case InlineOpenPGPFormat:
    return "inline openpgp";
  case OpenPGPMIMEFormat:
    return "openpgp/mime";
  case SMIMEFormat:
    return "s/mime";
  case SMIMEOpaqueFormat:
    return "s/mime opaque";
  };
}

QString Kleo::cryptoMessageFormatToLabel( Kleo::CryptoMessageFormat f ) {
  switch ( f ) {
  default:
    return QString::null;
  case AutoFormat:
    return i18n("Auto");
  case InlineOpenPGPFormat:
    return i18n("Inline OpenPGP (deprecated)");
  case OpenPGPMIMEFormat:
    return i18n("OpenPGP/MIME");
  case SMIMEFormat:
    return i18n("S/MIME");
  case SMIMEOpaqueFormat:
    return i18n("S/MIME opaque");
  }
}

Kleo::CryptoMessageFormat Kleo::stringToCryptoMessageFormat( const QString & s ) {
  const QString t = s.lower();
  if ( t == "inline openpgp" )
    return InlineOpenPGPFormat;
  if ( t == "openpgp/mime" )
    return OpenPGPMIMEFormat;
  if ( t == "s/mime" )
    return SMIMEFormat;
  if ( t == "s/mime opaque" )
    return SMIMEOpaqueFormat;
  return AutoFormat;
}
