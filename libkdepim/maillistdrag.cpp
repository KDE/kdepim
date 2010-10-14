/*
    This file is part of libkdepim.

    Copyright (c) 2003 Don Sanders <sanders@kde.org>
    Copyright (c) 2005 George Staikos <staikos@kde.org>

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

#include "maillistdrag.h"
#include <qbuffer.h>
#include <qdatastream.h>
#include <qeventloop.h>
#include <kapplication.h>
#include <klocale.h>
#include <kprogress.h>

using namespace KPIM;

MailSummary::MailSummary( Q_UINT32 serialNumber, QString messageId,
			  QString subject, QString from, QString to,
			  time_t date )
    : mSerialNumber( serialNumber ), mMessageId( messageId ),
      mSubject( subject ), mFrom( from ), mTo( to ), mDate( date )
{}

Q_UINT32 MailSummary::serialNumber() const
{
    return mSerialNumber;
}

QString MailSummary::messageId()
{
    return mMessageId;
}

QString MailSummary::subject()
{
    return mSubject;
}

QString MailSummary::from()
{
    return mFrom;
}

QString MailSummary::to()
{
    return mTo;
}

time_t MailSummary::date()
{
    return mDate;
}

void MailSummary::set( Q_UINT32 serialNumber, QString messageId,
		       QString subject, QString from, QString to, time_t date )
{
    mSerialNumber = serialNumber;
    mMessageId = messageId;
    mSubject = subject;
    mFrom = from;
    mTo = to;
    mDate = date;
}

MailListDrag::MailListDrag( const MailList& mailList, QWidget * parent, MailTextSource *src )
    : QDragObject( parent ), _src(src)
{
    setMailList( mailList );
}

MailListDrag::~MailListDrag()
{
    delete _src;
    _src = 0;
}

const char* MailListDrag::format()
{
    return "x-kmail-drag/message-list";
}

bool MailListDrag::canDecode( QMimeSource *e )
{
    return e->provides( MailListDrag::format() );
}

// Have to define before use
QDataStream& operator<< ( QDataStream &s, MailSummary &d )
{
    s << d.serialNumber();
    s << d.messageId();
    s << d.subject();
    s << d.from();
    s << d.to();
    s << d.date();
    return s;
}

QDataStream& operator>> ( QDataStream &s, MailSummary &d )
{
    Q_UINT32 serialNumber;
    QString messageId, subject, from, to;
    time_t date;
    s >> serialNumber;
    s >> messageId;
    s >> subject;
    s >> from;
    s >> to;
    s >> date;
    d.set( serialNumber, messageId, subject, from, to, date );
    return s;
}

QDataStream& operator<< ( QDataStream &s, const MailList &mailList )
{
    MailList::const_iterator it;
    for (it = mailList.begin(); it != mailList.end(); ++it) {
	MailSummary mailDrag = *it;
	s << mailDrag;
    }
    return s;
}

QDataStream& operator>> ( QDataStream &s, MailList &mailList )
{
    mailList.clear();
    MailSummary mailDrag;
    while (!s.atEnd()) {
	s >> mailDrag;
	mailList.append( mailDrag );
    }
    return s;
}

bool MailListDrag::decode( QDropEvent* e, MailList& mailList )
{
    QByteArray payload = e->encodedData( MailListDrag::format() );
    QDataStream buffer( payload, IO_ReadOnly );
    if ( payload.size() ) {
	e->accept();
	buffer >> mailList;
	return TRUE;
    }
    return FALSE;
}

bool MailListDrag::decode( QByteArray& payload, MailList& mailList )
{
    QDataStream stream( payload, IO_ReadOnly );
    if ( payload.size() ) {
	stream >> mailList;
	return TRUE;
    }
    return FALSE;
}

bool MailListDrag::decode( QDropEvent* e, QByteArray &a )
{
    MailList mailList;
    if (decode( e, mailList )) {
	MailList::iterator it;
	QBuffer buffer( a );
	buffer.open( IO_WriteOnly );
	QDataStream stream( &buffer );
	for (it = mailList.begin(); it != mailList.end(); ++it) {
	    MailSummary mailDrag = *it;
	    stream << mailDrag.serialNumber();
	}
	buffer.close();
	return TRUE;
    }
    return FALSE;
}

void MailListDrag::setMailList( const MailList& mailList )
{
    _mailList = mailList;
}

const char *MailListDrag::format(int i) const
{
  if (i == 0) {
    return "message/rfc822";
  } else if (i == 1) {
    return format();
  } else if (i == 2) {
    return "application/x-kde-suggestedfilename"; // issue3689
  } else {
    return 0;
  }
}

QByteArray MailListDrag::encodedData(const char *mimeType) const
{
  QByteArray array;

  if (!qstricmp(mimeType, format())) {
    QBuffer buffer( array );
    buffer.open( IO_WriteOnly);
    QDataStream stream( array, IO_WriteOnly );
    stream << _mailList;
    buffer.close();
  } else if (!qstricmp(mimeType, "application/x-kde-suggestedfilename")) {
    if (!_mailList.isEmpty()) {
      MailSummary firstMail = _mailList.first();
      array = firstMail.subject().utf8();
    }
  } else if (!qstricmp(mimeType, "message/rfc822")) {
    KProgressDialog *dlg = new KProgressDialog(0, 0, QString::null, i18n("Retrieving and storing messages..."), true);
    dlg->setAllowCancel(true);
    dlg->progressBar()->setTotalSteps(_mailList.count());
    int i = 0;
    dlg->progressBar()->setValue(i);
    dlg->show();

    QTextStream ts(array, IO_WriteOnly);
    for (MailList::ConstIterator it = _mailList.begin(); it != _mailList.end(); ++it) {
      MailSummary mailDrag = *it;
      ts << _src->text(mailDrag.serialNumber());
      if (dlg->wasCancelled()) {
        break;
      }
      dlg->progressBar()->setValue(++i);
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeSocketNotifiers);
    }

    delete dlg;
  }
  return array;
}
