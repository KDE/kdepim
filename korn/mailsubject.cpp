#include"mailsubject.h"

#include <kascii.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmime/kmime_codecs.h>

#include <QDateTime>
#include <QTextCodec>
#include <QVariant>

#include <ctype.h>

KornMailSubject::KornMailSubject() : _id(0), _drop(0), _size(-1), _date(-1), _fullMessage(false)
{
}

KornMailSubject::KornMailSubject(const QVariant &id, KMailDrop *drop)
	: _id( new QVariant( id ) ), _drop( drop ), _size(-1), _date(-1), _fullMessage(false)
{
}

KornMailSubject::KornMailSubject(const KornMailSubject & src)
	: _id(0), _drop(0), _size(-1), _date(-1)
{
	operator=(src);
}

KornMailSubject & KornMailSubject::operator= (const KornMailSubject & src)
{
	_size = src._size;
	_date = src._date;
	_subject = src._subject;
	_sender = src._sender;
	_header = src._header;
	_fullMessage = src._fullMessage;
	if (_id)
		delete _id;
	_id = 0;
	if (src._id)
		_id = new QVariant( *src._id );
	_drop = src._drop;
	return *this;
}

KornMailSubject::~KornMailSubject()
{
	if (_id)
		delete _id;
	_id = 0;
}

const QVariant KornMailSubject::getId() const
{
	return *_id;
}

QString KornMailSubject::toString() const
{
	QDateTime date;
	date.setTime_t(_date);
	return QString("KornMailSubject, Id: ") + (_id?_id->toString():QString("NULL")) + ", " + i18n("Subject:") + ' ' + _subject
		+ ", " + i18n("Sender:") + ' ' + _sender + ", " + i18n("Size:") + ' ' + QString::number(_size)
		+ ", " + i18n("Date:") + ' ' + date.toString(Qt::ISODate);
}

bool operator<( const KornMailSubject& sub1, const KornMailSubject& sub2 )
{
	return sub1.getDate() < sub2.getDate();
}

QString KornMailSubject::decodeRFC2047String(const QByteArray& aStr)
{
	if ( aStr.isEmpty() ) {
		return QString();
	}

	const QByteArray str = unfold( aStr );

	if ( str.isEmpty() ) {
		return QString();
	}

	if ( str.indexOf( "=?" ) < 0 ) {
		//QByteArray charsetName;
		//charsetName = GlobalSettings::self()->fallbackCharacterEncoding().toLatin1();
		//const QTextCodec *codec;// = KMMsgBase::codecForName( charsetName );
		//if ( ! codec ) {
			//codec = kmkernel->networkCodec();
		//}
		//return codec->toUnicode( str );
		return QString( aStr );
	}

	QString result;
	QByteArray LWSP_buffer;
	bool lastWasEncodedWord = false;

	for ( const char * pos = str.data() ; *pos ; ++pos ) {
		// collect LWSP after encoded-words,
		// because we might need to throw it out
		// (when the next word is an encoded-word)
		if ( lastWasEncodedWord && isBlank( pos[0] ) ) {
			LWSP_buffer += pos[0];
			continue;
		}
		// verbatimly copy normal text
		if (pos[0]!='=' || pos[1]!='?') {
			result += LWSP_buffer + pos[0];
			LWSP_buffer = 0;
			lastWasEncodedWord = false;
			continue;
		}
		// found possible encoded-word
		const char * const beg = pos;
		{
			// parse charset name
			QByteArray charset;
			int i = 2;
			pos += 2;
			for ( ; *pos != '?' && ( *pos==' ' || ispunct(*pos) || isalnum(*pos) ); ++i, ++pos ) {
				charset += *pos;
			}
			if ( *pos!='?' || i<4 )
				goto invalid_encoded_word;

			// get encoding and check delimiting question marks
			const char encoding[2] = { pos[1], '\0' };
			if (pos[2]!='?' || (encoding[0]!='Q' && encoding[0]!='q' && encoding[0]!='B' && encoding[0]!='b'))
				goto invalid_encoded_word;
			pos+=3; i+=3; // skip ?x?
			const char * enc_start = pos;
			// search for end of encoded part
			while ( *pos && !(*pos=='?' && *(pos+1)=='=') ) {
				i++;
				pos++;
			}
			if ( !*pos )
				goto invalid_encoded_word;

			// valid encoding: decode and throw away separating LWSP
			const KMime::Codec * c = KMime::Codec::codecForName( encoding );
			kFatal( !c, 5006 ) << "No \"" << encoding << "\" codec!?" << endl;

			QByteArray in = QByteArray::fromRawData( enc_start, pos - enc_start );
			const QByteArray enc = c->decode( in );
			in.clear();

			const QTextCodec * codec = codecForName(charset);
			//if (!codec) codec = kmkernel->networkCodec();
			if( codec )
				result += codec->toUnicode(enc);
			else
				result += QString( enc );
			lastWasEncodedWord = true;

			++pos; // eat '?' (for loop eats '=')
			LWSP_buffer = 0;
		}
		continue;
invalid_encoded_word:
		// invalid encoding, keep separating LWSP.
		pos = beg;
		if ( !LWSP_buffer.isNull() )
			result += LWSP_buffer;
		result += "=?";
		lastWasEncodedWord = false;
		++pos; // eat '?' (for loop eats '=')
		LWSP_buffer = 0;
	}
	return result;
}


QByteArray KornMailSubject::unfold(const QByteArray& header)
{
	if ( header.isEmpty() )
		return QByteArray();

	QByteArray result( header.size(), '\0' ); // size() >= length()+1 and size() is O(1)
	char * d = result.data();

	for ( const char * s = header.data() ; *s ; )
		if ( *s == '\r' ) { // ignore
			++s;
			continue;
		} else if ( *s == '\n' ) { // unfold
			while ( isBlank( *++s ) );
			*d++ = ' ';
		} else
			*d++ = *s++;

		*d++ = '\0';

	result.truncate( d - result.data() );
	return result;
}

const QTextCodec* KornMailSubject::codecForName(const QByteArray& _str)
{
	if (_str.isEmpty())
		return 0;
	QByteArray codec = _str;
	kAsciiToLower(codec.data());
	return KGlobal::charsets()->codecForName(codec);
}

void KornMailSubject::decodeHeaders()
{
	_subject = decodeRFC2047String( _subject.toAscii() );
	_sender = decodeRFC2047String( _sender.toAscii() );
}
