#include"mailsubject.h"

#include<kdebug.h>
#include <qdatetime.h>

KornMailSubject::KornMailSubject() : _id(0), _size(-1), _date(-1), _fullMessage(false)
{
}

KornMailSubject::KornMailSubject(KornMailId * id) : _id(id), _size(-1), _date(-1), _fullMessage(false)
{
}

KornMailSubject::KornMailSubject(const KornMailSubject & src)
	: _id(0), _size(-1), _date(-1)
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
		_id = src._id->clone();
	return *this;
}

KornMailSubject::~KornMailSubject()
{
	if (_id)
		delete _id;
	_id = 0;
}

QString KornMailSubject::toString() const
{
	QDateTime date;
	date.setTime_t(_date);
	return QString("KornMailSubject, Id: ") + (_id?_id->toString():QString("NULL")) + ", Subject: " + QString(_subject.utf8())
		+ ", Sender: " + QString(_sender.utf8()) + ", Size: " + QString::number(_size)
		+ ", Date: " + date.toString(Qt::ISODate);
}
