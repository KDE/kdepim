#include"mailsubject.h"

#include<kdebug.h>
#include <klocale.h>
#include <qdatetime.h>
#include <QVariant>

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
	return QString("KornMailSubject, Id: ") + (_id?_id->toString():QString("NULL")) + ", " + i18n("Subject:") + " " + _subject
		+ ", " + i18n("Sender:") + " " + _sender + ", " + i18n("Size:") + " " + QString::number(_size)
		+ ", " + i18n("Date:") + " " + date.toString(Qt::ISODate);
}

bool operator<( const KornMailSubject& sub1, const KornMailSubject& sub2 )
{
	return sub1.getDate() < sub2.getDate();
}

