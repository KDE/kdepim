/*
  qutf7codecplugin.cpp

  A TQTextCodec for UTF-7 (rfc2152).
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>
  See file COPYING for details

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, US

  As a special exception, permission is granted to use this plugin
  with any version of Qt by TrollTech AS, Norway. In this case, the
  use of this plugin doesn't cause the resulting executable to be
  covered by the GNU General Public License.
  This exception does not however invalidate any other reasons why the
  executable file might be covered by the GNU General Public License.
*/

#include "qutf7codec.h"

#include <tqtextcodecplugin.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>

class TQTextCodec;

// ######### This file isn't compiled currently

class QUtf7CodecPlugin : public TQTextCodecPlugin {
public:
  QUtf7CodecPlugin() {}

  TQStringList names() const { return TQStringList() << "UTF-7" << "X-QT-UTF-7-STRICT"; }
  TQValueList<int> mibEnums() const { return TQValueList<int>() << 1012 << -1012; }
  TQTextCodec * createForMib( int );
  TQTextCodec * createForName( const TQString & );
};

TQTextCodec * QUtf7CodecPlugin::createForMib( int mib ) {
  if ( mib == 1012 )
    return new QUtf7Codec();
  else if ( mib == -1012 )
    return new QStrictUtf7Codec();
  return 0;
}

TQTextCodec * QUtf7CodecPlugin::createForName( const TQString & name ) {
  if ( name == "UTF-7" )
    return new QUtf7Codec();
  else if ( name == "X-QT-UTF-7-STRICT" )
    return new QStrictUtf7Codec();
  return 0;
}

KDE_Q_EXPORT_PLUGIN( QUtf7CodecPlugin );
