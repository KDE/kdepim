/*
    ktnefwriter.cpp

    Copyright (C) 2002 Bo Thorsen  <bo@sonofthor.dk>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "ktnef/ktnefwriter.h"
#include "ktnef/ktnefproperty.h"
#include "ktnef/ktnefpropertyset.h"

#include <tqfile.h>
#include <tqdatetime.h>
#include <tqdatastream.h>
#include <kdebug.h>
#include <assert.h>

#include "ktnef/ktnefdefs.h"


class KTNEFWriter::PrivateData {
public:
  PrivateData() { mFirstAttachNum = TQDateTime::currentDateTime().toTime_t(); }

  KTNEFPropertySet properties;
  Q_UINT16 mFirstAttachNum;
};


KTNEFWriter::KTNEFWriter() {
  mData = new PrivateData;

  // This is not something the user should fiddle with
  // First set the TNEF version
  TQVariant v(0x00010000);
  addProperty( attTNEFVERSION, atpDWORD, v );

  // Now set the code page to something reasonable. TODO: Use the right one
  TQVariant v1( (Q_UINT32)0x4e4 );
  TQVariant v2( (Q_UINT32)0x0 );
  TQValueList<TQVariant> list;
  list << v1;
  list << v2;
  v = TQVariant( list );
  addProperty( attOEMCODEPAGE, atpBYTE, list );
}

KTNEFWriter::~KTNEFWriter() {
  delete mData;
}


void KTNEFWriter::addProperty( int tag, int type, const TQVariant& value ) {
  mData->properties.addProperty( tag, type, value );
}


void addToChecksum( Q_UINT32 i, Q_UINT16 &checksum ) {
  checksum += i & 0xff;
  checksum += (i >> 8) & 0xff;
  checksum += (i >> 16) & 0xff;
  checksum += (i >> 24) & 0xff;
}

void addToChecksum( TQCString &cs, Q_UINT16 &checksum ) {
  int len = cs.length();
  for (int i=0; i<len; i++)
    checksum += (Q_UINT8)cs[i];
}

void writeCString( TQDataStream &stream, TQCString &str ) {
  stream.writeRawBytes( str.data(), str.length() );
  stream << (Q_UINT8)0;
}

Q_UINT32 mergeTagAndType( Q_UINT32 tag, Q_UINT32 type ) {
  return ( ( type & 0xffff ) << 16 ) | ( tag & 0xffff );
}

/* This writes a TNEF property to the file.
 *
 * A TNEF property has a 1 byte type (LVL_MESSAGE or LVL_ATTACHMENT),
 * a 4 byte type/tag, a 4 byte length, the data and finally the checksum.
 *
 * The checksum is a 16 byte int with all bytes in the data added.
 */
bool KTNEFWriter::writeProperty( TQDataStream &stream, int &bytes, int tag) {
  TQMap<int,KTNEFProperty*>& properties = mData->properties.properties();
  TQMap<int,KTNEFProperty*>::Iterator it = properties.find( tag );

  if ( it == properties.end() )
    return false;

  KTNEFProperty *property = *it;

  Q_UINT32 i;
  Q_UINT16 checksum = 0;
  TQValueList<TQVariant> list;
  TQString s;
  TQCString cs, cs2;
  TQDateTime dt;
  TQDate date;
  TQTime time;
  switch( tag ) {
  case attMSGSTATUS:
    // Q_UINT8
    i = property->value().toUInt() & 0xff;
    checksum = i;

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)1;
    stream << (Q_UINT8)i;

    bytes += 10;
    break;

  case attMSGPRIORITY:
  case attREQUESTRES:
    // Q_UINT16
    i = property->value().toUInt() & 0xffff;
    addToChecksum( i, checksum );

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)2;
    stream << (Q_UINT16)i;

    bytes += 11;
    break;

  case attTNEFVERSION:
    // Q_UINT32
    i = property->value().toUInt();
    addToChecksum( i, checksum );

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)4;
    stream << (Q_UINT32)i;

    bytes += 13;
    break;

  case attOEMCODEPAGE:
    // 2 Q_UINT32
    list = property->value().toList();
    assert( list.count() == 2 );

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)8;

    i = list[0].toInt();
    addToChecksum( i, checksum );
    stream << (Q_UINT32)i;
    i = list[1].toInt();
    addToChecksum( i, checksum );
    stream << (Q_UINT32)i;

    bytes += 17;
    break;

  case attMSGCLASS:
  case attSUBJECT:
  case attBODY:
  case attMSGID:
    // QCString
    cs = property->value().toString().local8Bit();
    addToChecksum( cs, checksum );

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)cs.length()+1;
    writeCString( stream, cs );

    bytes += 9 + cs.length()+1;
    break;

  case attFROM:
    // 2 TQString encoded to a TRP structure
    list = property->value().toList();
    assert( list.count() == 2 );

    cs = list[0].toString().local8Bit();                       // Name
    cs2 = (TQString("smtp:") + list[1].toString()).local8Bit(); // Email address
    i = 18 + cs.length() + cs2.length(); // 2 * sizof(TRP) + strings + 2x'\0'

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)i;

    // The stream has to be aligned to 4 bytes for the strings
    // TODO: Or does it? Looks like Outlook doesn't do this
    // bytes += 17;
    // Write the first TRP structure
    stream << (Q_UINT16)4;                 // trpidOneOff
    stream << (Q_UINT16)i;                 // totalsize
    stream << (Q_UINT16)(cs.length()+1);   // sizeof name
    stream << (Q_UINT16)(cs2.length()+1);  // sizeof address

    // if ( bytes % 4 != 0 )
      // Align the buffer

    // Write the strings
    writeCString( stream, cs );
    writeCString( stream, cs2 );

    // Write the empty padding TRP structure (just zeroes)
    stream << (Q_UINT32)0 << (Q_UINT32)0;

    addToChecksum( 4, checksum );
    addToChecksum( i, checksum );
    addToChecksum( cs.length()+1, checksum );
    addToChecksum( cs2.length()+1, checksum );
    addToChecksum( cs, checksum );
    addToChecksum( cs2, checksum );

    bytes += 10;
    break;

  case attDATESENT:
  case attDATERECD:
  case attDATEMODIFIED:
    // QDateTime
    dt = property->value().toDateTime();
    time = dt.time();
    date = dt.date();

    stream << (Q_UINT8)LVL_MESSAGE;
    stream << mergeTagAndType( tag, property->type() );
    stream << (Q_UINT32)14;

    i = (Q_UINT16)date.year();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)date.month();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)date.day();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)time.hour();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)time.minute();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)time.second();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    i = (Q_UINT16)date.dayOfWeek();
    addToChecksum( i, checksum );
    stream << (Q_UINT16)i;
    break;
/*
  case attMSGSTATUS:
    {
      Q_UINT8 c;
      Q_UINT32 flag = 0;
      if ( c & fmsRead ) flag |= MSGFLAG_READ;
      if ( !( c & fmsModified ) ) flag |= MSGFLAG_UNMODIFIED;
      if ( c & fmsSubmitted ) flag |= MSGFLAG_SUBMIT;
      if ( c & fmsHasAttach ) flag |= MSGFLAG_HASATTACH;
      if ( c & fmsLocal ) flag |= MSGFLAG_UNSENT;
      d->stream_ >> c;

      i = property->value().toUInt();
      stream << (Q_UINT8)LVL_MESSAGE;
      stream << (Q_UINT32)type;
      stream << (Q_UINT32)2;
      stream << (Q_UINT8)i;
      addToChecksum( i, checksum );
      // from reader: d->message_->addProperty( 0x0E07, MAPI_TYPE_ULONG, flag );
    }
    kdDebug() << "Message Status" << " (length=" << i2 << ")" << endl;
    break;
*/

  default:
    kdDebug() << "Unknown TNEF tag: " << tag << endl;
    return false;
  }

  stream << (Q_UINT16)checksum;
  return true;
}


bool KTNEFWriter::writeFile( TQIODevice &file ) {
  if ( !file.open( IO_WriteOnly ) )
    return false;

  TQDataStream stream( &file );
  return writeFile( stream );
}


bool KTNEFWriter::writeFile( TQDataStream &stream ) {
  stream.setByteOrder( TQDataStream::LittleEndian );

  // Start by writing the opening TNEF stuff
  stream << TNEF_SIGNATURE;

  // Store the PR_ATTACH_NUM value for the first attachment
  // ( must be stored even if *no* attachments are stored )
  stream << mData->mFirstAttachNum;

  // Now do some writing
  bool ok = true;
  int bytesWritten = 0;
  ok &= writeProperty( stream, bytesWritten, attTNEFVERSION );
  ok &= writeProperty( stream, bytesWritten, attOEMCODEPAGE );
  ok &= writeProperty( stream, bytesWritten, attMSGCLASS );
  ok &= writeProperty( stream, bytesWritten, attMSGPRIORITY );
  ok &= writeProperty( stream, bytesWritten, attSUBJECT );
  ok &= writeProperty( stream, bytesWritten, attDATESENT );
  ok &= writeProperty( stream, bytesWritten, attDATESTART );
  ok &= writeProperty( stream, bytesWritten, attDATEEND );
  // ok &= writeProperty( stream, bytesWritten, attAIDOWNER );
  ok &= writeProperty( stream, bytesWritten, attREQUESTRES );
  ok &= writeProperty( stream, bytesWritten, attFROM );
  ok &= writeProperty( stream, bytesWritten, attDATERECD );
  ok &= writeProperty( stream, bytesWritten, attMSGSTATUS );
  ok &= writeProperty( stream, bytesWritten, attBODY );
  return ok;
}


void KTNEFWriter::setSender(const TQString &name, const TQString &email) {
  assert( !name.isEmpty() );
  assert( !email.isEmpty() );

  TQVariant v1( name );
  TQVariant v2( email );

  TQValueList<TQVariant> list;
  list << v1;
  list << v2;

  TQVariant v( list );
  addProperty( attFROM, 0, list ); // What's up with the 0 here ??
}

void KTNEFWriter::setMessageType(MessageType m) {
  // Note that the MessageType list here is probably not long enough,
  // more entries are most likely needed later

  TQVariant v;
  switch( m ) {
  case Appointment:
    v = TQVariant( TQString( "IPM.Appointment" ) );
    break;

  case MeetingCancelled:
    v = TQVariant( TQString( "IPM.Schedule.Meeting.Cancelled" ) );
    break;

  case MeetingRequest:
    v = TQVariant( TQString( "IPM.Schedule.Meeting.Request" ) );
    break;

  case MeetingNo:
    v = TQVariant( TQString( "IPM.Schedule.Meeting.Resp.Neg" ) );
    break;

  case MeetingYes:
    v = TQVariant( TQString( "IPM.Schedule.Meeting.Resp.Pos" ) );
    break;

  case MeetingTent:
    // Tent?
    v = TQVariant( TQString( "IPM.Schedule.Meeting.Resp.Tent" ) );
    break;

  default:
    return;
  }

  addProperty( attMSGCLASS, atpWORD, v );
}


void KTNEFWriter::setMethod( Method )
{

}


void KTNEFWriter::clearAttendees()
{

}


void KTNEFWriter::addAttendee( const TQString& /*cn*/, Role /*r*/, PartStat /*p*/,
			       bool /*rsvp*/, const TQString& /*mailto*/ )
{

}


// I assume this is the same as the sender?
// U also assume that this is like "Name <address>"
void KTNEFWriter::setOrganizer( const TQString& organizer ) {
  int i = organizer.find( '<' );

  if ( i == -1 )
    return;

  TQString name = organizer.left( i );
  name.stripWhiteSpace();

  TQString email = organizer.right( i+1 );
  email = email.left( email.length()-1 );
  email.stripWhiteSpace();

  setSender( name, email );
}


void KTNEFWriter::setDtStart( const TQDateTime& dtStart ) {
  TQVariant v( dtStart );
  addProperty( attDATESTART, atpDATE, v );
}


void KTNEFWriter::setDtEnd( const TQDateTime& dtEnd ) {
  TQVariant v( dtEnd );
  addProperty( attDATEEND, atpDATE, v );
}


void KTNEFWriter::setLocation( const TQString& /*location*/ )
{

}


void KTNEFWriter::setUID( const TQString& uid ) {
  TQVariant v( uid );
  addProperty( attMSGID, atpSTRING, v );
}


// Date sent
void KTNEFWriter::setDtStamp( const TQDateTime& dtStamp ) {
  TQVariant v( dtStamp );
  addProperty( attDATESENT, atpDATE, v );
}


void KTNEFWriter::setCategories( const TQStringList& )
{

}


// I hope this is the body
void KTNEFWriter::setDescription( const TQString &body ) {
  TQVariant v( body );
  addProperty( attBODY, atpTEXT, v );
}


void KTNEFWriter::setSummary( const TQString &s ) {
  TQVariant v( s );
  addProperty( attSUBJECT, atpSTRING, v );
}

// TNEF encoding: Normal =  3, high = 2, low = 1
// MAPI encoding: Normal = -1, high = 0, low = 1
void KTNEFWriter::setPriority( Priority p ) {
  TQVariant v( (Q_UINT32)p );
  addProperty( attMSGPRIORITY, atpSHORT, v );
}


void KTNEFWriter::setAlarm( const TQString& /*description*/, AlarmAction /*action*/,
                            const TQDateTime& /*wakeBefore*/ )
{

}
