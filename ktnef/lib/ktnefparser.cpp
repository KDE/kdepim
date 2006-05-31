/*
    ktnefparser.cpp

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <config-ktnef.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "ktnef/ktnefparser.h"
#include "ktnef/ktnefattach.h"
#include "ktnef/ktnefproperty.h"
#include "ktnef/ktnefmessage.h"

#include <QDateTime>
#include <QDataStream>
#include <QFile>
#include <QVariant>
#include <QList>
#include <kdebug.h>
#include <kmimetype.h>
#include <ksavefile.h>

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#include "ktnef/ktnefdefs.h"


typedef struct {
	quint16 type;
	quint16 tag;
	QVariant value;
	struct {
		quint32 type;
		QVariant value;
	} name;
} MAPI_value;

void clearMAPIName( MAPI_value& mapi );
void clearMAPIValue(MAPI_value& mapi, bool clearName = true);
QString readMAPIString( QDataStream& stream, bool isUnicode = false, bool align = true, int len = -1 );
quint16 readMAPIValue(QDataStream& stream, MAPI_value& mapi);
QDateTime readTNEFDate( QDataStream& stream );
QString readTNEFAddress( QDataStream& stream );
QByteArray readTNEFData( QDataStream& stream, quint32 len );
QVariant readTNEFAttribute( QDataStream& stream, quint16 type, quint32 len );
QDateTime formatTime( quint32 lowB, quint32 highB );
QString formatRecipient( const QMap<int,KTNEFProperty*>& props );

//------------------------------------------------------------------------------------

class KTNEFParser::ParserPrivate
{
public:
	ParserPrivate()
	{
		defaultdir_ = "/tmp/";
		current_ = 0;
		deleteDevice_ = false;
		device_ = 0;
		message_ = new KTNEFMessage;
	}
	~ParserPrivate()
	{
		delete message_;
	}

	QDataStream              stream_;
	QIODevice                *device_;
	bool                     deleteDevice_;
	QString                  defaultdir_;
	KTNEFAttach              *current_;
	KTNEFMessage             *message_;
};

KTNEFParser::KTNEFParser()
{
	d = new ParserPrivate;
}

KTNEFParser::~KTNEFParser()
{
	deleteDevice();
	delete d;
}

KTNEFMessage* KTNEFParser::message() const
{
	return d->message_;
}

void KTNEFParser::deleteDevice()
{
	if ( d->deleteDevice_ )
		delete d->device_;
	d->device_ = 0;
	d->deleteDevice_ = false;
}

bool KTNEFParser::decodeMessage()
{
	quint32	i1, i2, off;
	quint16	u, tag, type;
	QVariant value;

	// read (type+name)
	d->stream_ >> i1;
        u = 0;
	tag = ( i1 & 0x0000FFFF );
	type = ( ( i1 & 0xFFFF0000 ) >> 16 );
	// read data length
	d->stream_ >> i2;
	// offset after reading the value
	off = d->device_->pos() + i2;
	switch ( tag )
	{
		case attAIDOWNER: {
			uint tmp;
			d->stream_ >> tmp;
			value.setValue( tmp );
			d->message_->addProperty( 0x0062, MAPI_TYPE_ULONG, value );
			kDebug() << "Message Owner Appointment ID" << " (length=" << i2 << ")" << endl;
			break;}
		case attREQUESTRES:
			d->stream_ >> u;
			d->message_->addProperty( 0x0063, MAPI_TYPE_UINT16, u );
			value = ( bool )u;
			kDebug() << "Message Request Response" << " (length=" << i2 << ")" << endl;
			break;
		case attDATERECD:
			value = readTNEFDate( d->stream_ );
			d->message_->addProperty( 0x0E06, MAPI_TYPE_TIME, value );
			kDebug() << "Message Receive Date" << " (length=" << i2 << ")" << endl;
			break;
		case attMSGCLASS:
			value = readMAPIString( d->stream_, false, false, i2 );
			d->message_->addProperty( 0x001A, MAPI_TYPE_STRING8, value );
			kDebug() << "Message Class" << " (length=" << i2 << ")" << endl;
			break;
		case attMSGPRIORITY:
			d->stream_ >> u;
			d->message_->addProperty( 0x0026, MAPI_TYPE_ULONG, 2-u );
			value = u;
			kDebug() << "Message Priority" << " (length=" << i2 << ")" << endl;
			break;
		case attMAPIPROPS:
			kDebug() << "Message MAPI Properties" << " (length=" << i2 << ")" << endl;
			{
				int nProps = d->message_->properties().count();
				i2 += d->device_->pos();
				readMAPIProperties( d->message_->properties(), 0 );
				d->device_->seek( i2 );
				kDebug() << "Properties: " << d->message_->properties().count() << endl;
				value = QString( "< %1 properties >" ).arg( d->message_->properties().count() - nProps );
			}
			break;
		case attTNEFVERSION: {
			uint tmp;
			d->stream_ >> tmp;
			value.setValue( tmp );
			kDebug() << "Message TNEF Version" << " (length=" << i2 << ")" << endl;}
			break;
		case attFROM:
			d->message_->addProperty( 0x0024, MAPI_TYPE_STRING8, readTNEFAddress( d->stream_ ) );
			d->device_->seek( d->device_->pos() - i2 );
			value = readTNEFData( d->stream_, i2 );
			kDebug() << "Message From" << " (length=" << i2 << ")" << endl;
			break;
		case attSUBJECT:
			value = readMAPIString( d->stream_, false, false, i2 );
			d->message_->addProperty( 0x0037, MAPI_TYPE_STRING8, value );
			kDebug() << "Message Subject" << " (length=" << i2 << ")" << endl;
			break;
		case attDATESENT:
			value = readTNEFDate( d->stream_ );
			d->message_->addProperty( 0x0039, MAPI_TYPE_TIME, value );
			kDebug() << "Message Date Sent" << " (length=" << i2 << ")" << endl;
			break;
		case attMSGSTATUS:
			{
				quint8 c;
				quint32 flag = 0;
				d->stream_ >> c;
				if ( c & fmsRead ) flag |= MSGFLAG_READ;
				if ( !( c & fmsModified ) ) flag |= MSGFLAG_UNMODIFIED;
				if ( c & fmsSubmitted ) flag |= MSGFLAG_SUBMIT;
				if ( c & fmsHasAttach ) flag |= MSGFLAG_HASATTACH;
				if ( c & fmsLocal ) flag |= MSGFLAG_UNSENT;
				d->message_->addProperty( 0x0E07, MAPI_TYPE_ULONG, flag );
				value = c;
			}
			kDebug() << "Message Status" << " (length=" << i2 << ")" << endl;
			break;
		case attRECIPTABLE:
			{
				quint32 rows;
				QList<QVariant> recipTable;
				d->stream_ >> rows;
				for ( uint i=0; i<rows; i++ )
				{
					QMap<int,KTNEFProperty*> props;
					readMAPIProperties( props, 0 );
					recipTable << formatRecipient( props );
				}
				d->message_->addProperty( 0x0E12, MAPI_TYPE_STRING8, recipTable );
				d->device_->seek( d->device_->pos() - i2 );
				value = readTNEFData( d->stream_, i2 );
			}
			kDebug() << "Message Recipient Table" << " (length=" << i2 << ")" << endl;
			break;
		case attBODY:
			value = readMAPIString( d->stream_, false, false, i2 );
			d->message_->addProperty( 0x1000, MAPI_TYPE_STRING8, value );
			kDebug() << "Message Body" << " (length=" << i2 << ")" << endl;
			break;
		case attDATEMODIFIED:
			value = readTNEFDate( d->stream_ );
			d->message_->addProperty( 0x3008, MAPI_TYPE_TIME, value );
			kDebug() << "Message Date Modified" << " (length=" << i2 << ")" << endl;
			break;
		case attMSGID:
			value = readMAPIString( d->stream_, false, false, i2 );
			d->message_->addProperty( 0x300B, MAPI_TYPE_STRING8, value );
			kDebug() << "Message ID" << " (length=" << i2 << ")" << endl;
			break;
		case attOEMCODEPAGE:
			value = readTNEFData( d->stream_, i2 );
			kDebug() << "Message OEM Code Page" << " (length=" << i2 << ")" << endl;
			break;
		default:
			value = readTNEFAttribute( d->stream_, type, i2 );
			kDebug().form( "Message: type=%x, length=%d, check=%x\n", i1, i2, u );
			break;
	}
	// skip data
	if ( d->device_->pos() != off && !d->device_->seek( off ) )
		return false;
	// get checksum
	d->stream_ >> u;
	// add TNEF attribute
	d->message_->addAttribute( tag, type, value, true );
	//kDebug() << "stream: " << d->device_->pos() << endl;
	return true;
}

bool KTNEFParser::decodeAttachment()
{
	quint32	i;
	quint16	tag, type, u;
	QVariant value;
	QString str;

	d->stream_ >> i;		// i <- attribute type & name
	tag = ( i & 0x0000FFFF );
	type = ( ( i & 0xFFFF0000 ) >> 16 );
	d->stream_ >> i;		// i <- data length
	checkCurrent( tag );
	switch (tag)
	{
	   case attATTACHTITLE:
		   value = readMAPIString( d->stream_, false, false, i );
		   d->current_->setName( value.toString() );
		   kDebug() << "Attachment Title: " << d->current_->name() << endl;
		   break;
	   case attATTACHDATA:
		   d->current_->setSize( i );
		   d->current_->setOffset( d->device_->pos() );
		   d->device_->seek( d->device_->pos() + i );
		   value = QString( "< size=%1 >" ).arg( i );
		   kDebug() << "Attachment Data: size=" << i << endl;
		   break;
	   case attATTACHMENT:	// try to get attachment info
		   i += d->device_->pos();
		   readMAPIProperties( d->current_->properties(), d->current_ );
		   d->device_->seek( i );
		   d->current_->setIndex( d->current_->property( MAPI_TAG_INDEX ).toUInt() );
		   d->current_->setDisplaySize( d->current_->property( MAPI_TAG_SIZE ).toUInt() );
		   str = d->current_->property( MAPI_TAG_DISPLAYNAME ).toString();
		   if ( !str.isEmpty() )
			   d->current_->setDisplayName( str );
		   d->current_->setFileName( d->current_->property( MAPI_TAG_FILENAME ).toString() );
		   str = d->current_->property( MAPI_TAG_MIMETAG ).toString();
		   if ( !str.isEmpty() )
			   d->current_->setMimeTag( str );
		   d->current_->setExtension( d->current_->property( MAPI_TAG_EXTENSION ).toString() );
		   value = QString( "< %1 properties >" ).arg( d->current_->properties().count() );
		   break;
	   case attATTACHMODDATE:
		   value = readTNEFDate( d->stream_ );
		   kDebug() << "Attachment Modification Date: " << value.toString() << endl;
		   break;
	   case attATTACHCREATEDATE:
		   value = readTNEFDate( d->stream_ );
		   kDebug() << "Attachment Creation Date: " << value.toString() << endl;
		   break;
	   case attATTACHMETAFILE:
		   kDebug() << "Attachment Metafile: size=" << i << endl;
		   //value = QString( "< size=%1 >" ).arg( i );
		   //d->device_->seek( d->device_->pos()+i );
		   value = readTNEFData( d->stream_, i );
		   break;
	   default:
		   value = readTNEFAttribute( d->stream_, type, i );
		   kDebug().form( "Attachment unknown field:         tag=%x, length=%d\n",  tag, i);
		   break;
	}
	d->stream_ >> u;	// u <- checksum
	// add TNEF attribute
	d->current_->addAttribute( tag, type, value, true );
	//kDebug() << "stream: " << d->device_->pos() << endl;

	return true;
}

void KTNEFParser::setDefaultExtractDir(const QString& dirname)
{
	d->defaultdir_ = dirname;
}

bool KTNEFParser::parseDevice()
{
	quint16	u;
	quint32	i;
	quint8		c;

	d->message_->clearAttachments();
	if (d->current_)
	{
		delete d->current_;
		d->current_ = 0;
	}

	if ( !d->device_->open( QIODevice::ReadOnly ) ) {
	  kDebug() << "Couldn't open device" << endl;
		return false;
	}

	d->stream_.setDevice( d->device_ );
	d->stream_.setByteOrder( QDataStream::LittleEndian );
	d->stream_ >> i;
	if (i == TNEF_SIGNATURE)
	{
		d->stream_ >> u;
		kDebug().form( "Attachment cross reference key: 0x%04x\n",u );
		//kDebug() << "stream: " << d->device_->pos() << endl;
		while (!d->stream_.atEnd())
		{
			d->stream_ >> c;
			switch (c)
			{
			   case LVL_MESSAGE:
				if (!decodeMessage()) goto end;
				break;
			   case LVL_ATTACHMENT:
				if (!decodeAttachment()) goto end;
				break;
			   default:
				kDebug() << "Unknown Level: " << c << ", at offset " << d->device_->pos() << endl;
				goto end;
			}
		}
		if (d->current_)
		{
			checkCurrent(attATTACHDATA);	// this line has the effect to append the
								// attachment, if it has data. If not it does
								// nothing, and the attachment will be discarded
			delete d->current_;
			d->current_ = 0;
		}
		return true;
	}
	else
	{
	  kDebug() << "This is not a TNEF file" << endl;
end:	d->device_->close();
		return false;
	}
}

bool KTNEFParser::extractFile(const QString& filename)
{
	KTNEFAttach	*att = d->message_->attachment(filename);
	if (!att) return false;
	return extractAttachmentTo(att, d->defaultdir_);
}

bool KTNEFParser::extractAttachmentTo(KTNEFAttach *att, const QString& dirname)
{
	QString	filename = dirname + "/" + att->name();
	if (!d->device_->isOpen())
		return false;
	if (!d->device_->seek(att->offset()))
		return false;
	KSaveFile saveFile( filename );
	QFile *outfile = saveFile.file();
	if ( !outfile )
		return false;

	quint32	len = att->size(), sz(16384);
	int		n(0);
	char		*buf = new char[sz];
	bool		ok(true);
	while (ok && len > 0)
	{
		n = d->device_->read(buf,qMin(sz,len));
		if (n < 0)
			ok = false;
		else
		{
			len -= n;
			if (outfile->write(buf,n) != n)
				ok = false;
		}
	}
	delete [] buf;

	return ok;
}

bool KTNEFParser::extractAll()
{
  QList<KTNEFAttach*> l = d->message_->attachmentList();
  QList<KTNEFAttach*>::const_iterator it = l.begin();
	for ( ; it != l.end(); ++it )
		if (!extractAttachmentTo(*it,d->defaultdir_)) return false;
	return true;
}

bool KTNEFParser::extractFileTo(const QString& filename, const QString& dirname)
{
	kDebug() << "Extracting attachment: filename=" << filename << ", dir=" << dirname << endl;
	KTNEFAttach	*att = d->message_->attachment(filename);
	if (!att) return false;
	return extractAttachmentTo(att, dirname);
}

bool KTNEFParser::openFile(const QString& filename)
{
	deleteDevice();
	d->device_ = new QFile( filename );
	d->deleteDevice_ = true;
	return parseDevice();
}

bool KTNEFParser::openDevice( QIODevice *device )
{
	deleteDevice();
	d->device_ = device;
	return parseDevice();
}

void KTNEFParser::checkCurrent( int key )
{
	if ( !d->current_ )
		d->current_ = new KTNEFAttach();
	else
	{
		if ( d->current_->attributes().contains( key ) )
		{
			if (d->current_->offset() >= 0 )
			{
				if (d->current_->name().isEmpty())
					d->current_->setName("Unnamed");
				if ( d->current_->mimeTag().isEmpty() )
				{
					// No mime type defined in the TNEF structure,
					// try to find it from the attachment filename
					// and/or content (using at most 32 bytes)
					KMimeType::Ptr mimetype;
					if ( !d->current_->fileName().isEmpty() )
						mimetype = KMimeType::findByPath( d->current_->fileName(), 0, true );
                                        if (!mimetype) return; // FIXME
					if ( mimetype->name() == "application/octet-stream" && d->current_->size() > 0 )
					{
						int oldOffset = d->device_->pos();
						QByteArray buffer( qMin( 32, d->current_->size() ), '\0' );
						d->device_->seek( d->current_->offset() );
						d->device_->read( buffer.data(), buffer.size() );
						mimetype = KMimeType::findByContent( buffer );
						d->device_->seek( oldOffset );
					}
					d->current_->setMimeTag( mimetype->name() );
				}
				d->message_->addAttachment( d->current_ );
				d->current_ = 0;
			}
			else
			{ // invalid attachment, skip it
				delete d->current_;
				d->current_ = 0;
			}
			d->current_ = new KTNEFAttach();
		}
	}
}

//----------------------------------------------------------------------------------------

#define ALIGN( n, b ) if ( n & ( b-1 ) ) { n = ( n + b ) & ~( b-1 ); }
#define ISVECTOR( m ) ( ( ( m ).type & 0xF000 ) == MAPI_TYPE_VECTOR )

void clearMAPIName( MAPI_value& mapi )
{
	mapi.name.value.clear();
}

void clearMAPIValue(MAPI_value& mapi, bool clearName)
{
	mapi.value.clear();
	if ( clearName )
		clearMAPIName( mapi );
}

QDateTime formatTime( quint32 lowB, quint32 highB )
{
	QDateTime dt;
#if ( SIZEOF_UINT64_T == 8 )
	uint64_t u64;
#elif ( SIZEOF_UNSIGNED_LONG_LONG == 8 )
	unsigned long long u64;
#elif ( SIZEOF_UNSIGNED_LONG == 8 )
	unsigned long u64;
#else
	kWarning() << "Unable to perform date conversion on this system, no 64-bits integer found" << endl;
	dt.setTime_t( 0xffffffffU );
	return dt;
#endif
	u64 = highB;
	u64 <<= 32;
	u64 |= lowB;
	u64 -= 116444736000000000LL;
	u64 /= 10000000;
	if ( u64 <= 0xffffffffU )
		dt.setTime_t( ( unsigned int )u64 );
	else
	{
		kWarning().form( "Invalid date: low byte=0x%08X, high byte=0x%08X\n", lowB, highB );
		dt.setTime_t( 0xffffffffU );
	}
	return dt;
}

QString formatRecipient( const QMap<int,KTNEFProperty*>& props )
{
	QString s, dn, addr, t;
	QMap<int,KTNEFProperty*>::ConstIterator it;
	if ( ( it = props.find( 0x3001 ) ) != props.end() )
		dn = ( *it )->valueString();
	if ( ( it = props.find( 0x3003 ) ) != props.end() )
		addr = ( *it )->valueString();
	if ( ( it = props.find( 0x0C15 ) ) != props.end() )
		switch ( ( *it )->value().toInt() )
		{
			case 0: t = "From:"; break;
			case 1: t = "To:"; break;
			case 2: t = "Cc:"; break;
			case 3: t = "Bcc:"; break;
		}

	if ( !t.isEmpty() )
		s.append( t );
	if ( !dn.isEmpty() )
		s.append( " " + dn );
	if ( !addr.isEmpty() && addr != dn )
		s.append( " <" + addr + ">" );

	return s.trimmed();
}

QDateTime readTNEFDate( QDataStream& stream )
{
	// 14-bytes long
	quint16 y, m, d, hh, mm, ss, dm;
	stream >> y >> m >> d >> hh >> mm >> ss >> dm;
	return QDateTime( QDate( y, m, d ), QTime( hh, mm, ss ) );
}

QString readTNEFAddress( QDataStream& stream )
{
	quint16 totalLen, strLen, addrLen;
	QString s;
	stream >> totalLen >> totalLen >> strLen >> addrLen;
	s.append( readMAPIString( stream, false, false, strLen ) );
	s.append( " <" );
	s.append( readMAPIString( stream, false, false, addrLen ) );
	s.append( ">" );
	quint8 c;
	for ( int i=8+strLen+addrLen; i<totalLen; i++ )
		stream >> c;
	return s;
}

QByteArray readTNEFData( QDataStream& stream, quint32 len )
{
	QByteArray array( len, '\0' );
	if ( len > 0 )
		stream.readRawData( array.data(), len );
	return array;
}

QVariant readTNEFAttribute( QDataStream& stream, quint16 type, quint32 len )
{
	switch ( type )
	{
		case atpTEXT:
		case atpSTRING:
			return readMAPIString( stream, false, false, len );
		case atpDATE:
			return readTNEFDate( stream );
		default:
			return readTNEFData( stream, len );
	}
}

QString readMAPIString( QDataStream& stream, bool isUnicode, bool align, int len_ )
{
	quint32 len;
	char *buf = 0;
	if ( len_ == -1 )
		stream >> len;
	else
		len = len_;
	quint32 fullLen = len;
	if ( align )
		ALIGN( fullLen, 4 );
	buf = new char[ len ];
	stream.readRawData( buf, len );
	quint8 c;
	for ( uint i=len; i<fullLen; i++ )
		stream >> c;
	QString res;
	if ( isUnicode )
		res = QString::fromUtf16( ( const unsigned short* )buf );
	else
		res = QString::fromLocal8Bit( buf );
	delete [] buf;
	return res;
}

quint16 readMAPIValue(QDataStream& stream, MAPI_value& mapi)
{
	quint32	d;

	clearMAPIValue(mapi);
	stream >> d;
	mapi.type =  (d & 0x0000FFFF);
	mapi.tag = ((d & 0xFFFF0000) >> 16);
	if ( mapi.tag >= 0x8000 && mapi.tag <= 0xFFFE )
	{
		// skip GUID
		stream >> d >> d >> d >> d;
		// name type
		stream >> mapi.name.type;
		// name
		if ( mapi.name.type == 0 ) {
			uint tmp;
			stream >> tmp;
			mapi.name.value.setValue( tmp );
		} else if ( mapi.name.type == 1 ) {
			mapi.name.value.setValue( readMAPIString( stream, true ) );
		}
	}

	int n = 1;
	QVariant value;
	if ( ISVECTOR( mapi ) )
	{
		stream >> n;
		mapi.value = QList<QVariant>();
	}
	for ( int i=0; i<n; i++ )
	{
		value.clear();
		switch(mapi.type & 0x0FFF)
		{
		   case MAPI_TYPE_UINT16:
			stream >> d;
			value.setValue( d & 0x0000FFFF );
			break;
		   case MAPI_TYPE_BOOLEAN:
		   case MAPI_TYPE_ULONG: {
		   uint tmp;
		   stream >> tmp;
			value.setValue( tmp );
			}
			break;
		   case MAPI_TYPE_FLOAT:
			 // FIXME: Don't we have to set the value here
			stream >> d;
			break;
		   case MAPI_TYPE_DOUBLE: {
		   double tmp;
		   stream >> tmp;
			value.setValue( tmp );
			}
			break;
		   case MAPI_TYPE_TIME:
			{
				quint32 lowB, highB;
				stream >> lowB >> highB;
				value = formatTime( lowB, highB );
			}
			break;
		   case MAPI_TYPE_STRING8:
			// in case of a vector'ed value, the number of elements
			// has already been read in the upper for-loop
			if ( ISVECTOR( mapi ) )
				d = 1;
			else
				stream >> d;
			for (uint i=0;i<d;i++)
			{
				value.clear();
				value.setValue( readMAPIString( stream ) );
			}
			break;
		   case MAPI_TYPE_USTRING:
			mapi.type = MAPI_TYPE_NONE;
			break;
		   case MAPI_TYPE_OBJECT:
		   case MAPI_TYPE_BINARY:
			if ( ISVECTOR( mapi ) )
				d = 1;
			else
				stream >> d;
			for (uint i=0;i<d;i++)
			{
				value.clear();
				quint32 len;
				stream >> len;
				value = QByteArray( len, '\0' );
				if (len > 0)
				{
					int fullLen = len;
					ALIGN(fullLen, 4);
					stream.readRawData(value.toByteArray().data(), len);
					quint8 c;
					for ( int i=len; i<fullLen; i++ )
						stream >> c;
					// FIXME: Shouldn't we do something with the value???
				}
			}
			break;
		   default:
			mapi.type = MAPI_TYPE_NONE;
			break;
		}
		if ( ISVECTOR( mapi ) ) {
		  QList <QVariant> lst = mapi.value.toList();
		  lst << value;
			mapi.value.setValue( lst );
		} else {
			mapi.value = value;
		}
	}
	return mapi.tag;
}

bool KTNEFParser::readMAPIProperties( QMap<int,KTNEFProperty*>& props, KTNEFAttach *attach )
{
	quint32	n;
	MAPI_value	mapi;
	KTNEFProperty *p;
	QMap<int,KTNEFProperty*>::ConstIterator it;

	// some initializations
	mapi.type = MAPI_TYPE_NONE;
	mapi.value.clear();

	// get number of properties
	d->stream_ >> n;
	kDebug() << "MAPI Properties: " << n << endl;
	for (uint i=0;i<n;i++)
	{
		if (d->stream_.atEnd())
		{
			clearMAPIValue(mapi);
			return false;
		}
		readMAPIValue(d->stream_, mapi);
		if (mapi.type == MAPI_TYPE_NONE)
		{
			kDebug().form( "MAPI unsupported:         tag=%x, type=%x\n", mapi.tag, mapi.type );
			clearMAPIValue(mapi);
			return false;
		}
		int key = mapi.tag;
		switch (mapi.tag)
		{
		   case MAPI_TAG_DATA:
			{
				if ( mapi.type == MAPI_TYPE_OBJECT && attach )
				{
					QByteArray data = mapi.value.toByteArray();
					int len = data.size();
					ALIGN( len, 4 );
					d->device_->seek( d->device_->pos()-len );
					quint32 interface_ID;
					d->stream_ >> interface_ID;
					if ( interface_ID == MAPI_IID_IMessage )
					{
						// embedded TNEF file
						attach->unsetDataParser();
						attach->setOffset( d->device_->pos()+12 );
						attach->setSize( data.size()-16 );
						attach->setMimeTag( "application/ms-tnef" );
						attach->setDisplayName( "Embedded Message" );
						kDebug() << "MAPI Embedded Message: size=" << data.size() << endl;
					}
					d->device_->seek( d->device_->pos() + ( len-4 ) );
					break;
				}
			}
			kDebug().form( "MAPI data: size=%d\n", mapi.value.toByteArray().size() );
			break;
		   default:
			{
				QString mapiname = "";
				if ( mapi.tag >= 0x8000 && mapi.tag <= 0xFFFE )
				{
					if ( mapi.name.type == 0 )
						mapiname = QString().sprintf( " [name = 0x%04x]", mapi.name.value.toUInt() );
					else
						mapiname = QString( " [name = %1]" ).arg( mapi.name.value.toString() );
				}
				switch ( mapi.type & 0x0FFF )
				{
					case MAPI_TYPE_UINT16:
						kDebug().form( "(tag=%04x) MAPI short%s: 0x%x\n", mapi.tag, mapiname.toAscii().data(), mapi.value.toUInt() );
						break;
					case MAPI_TYPE_ULONG:
						kDebug().form( "(tag=%04x) MAPI long%s: 0x%x\n", mapi.tag, mapiname.toAscii().data(), mapi.value.toUInt() );
						break;
					case MAPI_TYPE_BOOLEAN:
						kDebug().form( "(tag=%04x) MAPI boolean%s: %s\n", mapi.tag, mapiname.toAscii().data(), ( mapi.value.toBool() ? "true" : "false" ) );
						break;
					case MAPI_TYPE_TIME:
						kDebug().form( "(tag=%04x) MAPI time%s: %s\n", mapi.tag, mapiname.toAscii().data(), mapi.value.toString().toAscii().data() );
						break;
					case MAPI_TYPE_USTRING:
					case MAPI_TYPE_STRING8:
						kDebug().form( "(tag=%04x) MAPI string%s: size=%d \"%s\"\n", mapi.tag, mapiname.toAscii().data(), mapi.value.toByteArray().size(), mapi.value.toString().toAscii().data() );
						break;
					case MAPI_TYPE_BINARY:
						kDebug().form( "(tag=%04x) MAPI binary%s: size=%d\n", mapi.tag, mapiname.toAscii().data(), mapi.value.toByteArray().size() );
						break;
				}
			}
			break;
		}
		// do not remove potential existing similar entry
		if ( ( it = props.find( key ) ) == props.end() )
		{
			p = new KTNEFProperty( key, ( mapi.type & 0x0FFF ), mapi.value, mapi.name.value );
			props[ p->key() ] = p;
		}
		//kDebug() << "stream: " << d->device_->pos() << endl;
	}
	return true;
}
