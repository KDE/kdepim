/*
    This file is part of libkdepim.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kvcarddrag.h"

#include <kabc/vcardconverter.h>

static const char vcard_mime_string[] = "text/x-vcard";

#if defined(KABC_VCARD_ENCODING_FIX)
KVCardDrag::KVCardDrag( const QByteArray &content, QWidget *dragsource, const char *name )
#else
KVCardDrag::KVCardDrag( const QString &content, QWidget *dragsource, const char *name )
#endif
  : QStoredDrag( vcard_mime_string, dragsource, name )
{
  setVCard( content );
}

KVCardDrag::KVCardDrag( QWidget *dragsource, const char *name )
  : QStoredDrag( vcard_mime_string, dragsource, name )
{
#if defined(KABC_VCARD_ENCODING_FIX)
  setVCard( QByteArray() );
#else
  setVCard( QString::null );
#endif
}

#if defined(KABC_VCARD_ENCODING_FIX)
void KVCardDrag::setVCard( const QByteArray &content )
{
  setEncodedData( content );
}
#else
void KVCardDrag::setVCard( const QString &content )
{
  setEncodedData( content.utf8() );
}
#endif

bool KVCardDrag::canDecode( QMimeSource *e )
{
  return e->provides( vcard_mime_string );
}

#if defined(KABC_VCARD_ENCODING_FIX)
bool KVCardDrag::decode( QMimeSource *e, QByteArray &content )
{
  if ( !canDecode( e ) ) {
    return false;
  }
  content = e->encodedData( vcard_mime_string );
  return true;
}
#else
bool KVCardDrag::decode( QMimeSource *e, QString &content )
{
  if ( !canDecode( e ) ) {
    return false;
  }
  content = QString::fromUtf8( e->encodedData( vcard_mime_string ) );
  return true;
}
#endif

bool KVCardDrag::decode( QMimeSource *e, KABC::Addressee::List& addressees )
{
  if ( !canDecode( e ) ) {
    return false;
  }
#if defined(KABC_VCARD_ENCODING_FIX)
  addressees = KABC::VCardConverter().parseVCardsRaw( e->encodedData( vcard_mime_string ).data() );
#else
  addressees = KABC::VCardConverter().parseVCards( e->encodedData( vcard_mime_string ) );
#endif
  return true;
}

void KVCardDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kvcarddrag.moc"
