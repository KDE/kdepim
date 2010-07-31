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
#include <tqbuffer.h>
#include <tqdatastream.h>
#include <tqeventloop.h>
#include <kapplication.h>
#include <klocale.h>
#include <kprogress.h>

using namespace KPIM;

MailSummary::MailSummary( Q_UINT32 serialNumber, TQString messageId, 
			  TQString subject, TQString from, TQString to, 
			  time_t date )
    : mSerialNumber( serialNumber ), mMessageId( messageId ),
      mSubject( subject ), mFrom( from ), mTo( to ), mDate( date )
{}

Q_UINT32 MailSummary::serialNumber() const 
{ 
    return mSerialNumber; 
}

TQString MailSummary::messageId() 
{ 
    return mMessageId; 
}

TQString MailSummary::subject() 
{ 
    return mSubject; 
}

TQString MailSummary::from() 
{ 
    return mFrom; 
}

TQString MailSummary::to() 
{ 
    return mTo; 
}

time_t MailSummary::date()
{
    return mDate;
}

void MailSummary::set( Q_UINT32 serialNumber, TQString messageId, 
		       TQString subject, TQString from, TQString to, time_t date )
{
    mSerialNumber = serialNumber;
    mMessageId = messageId;
    mSubject = subject;
    mFrom = from;
    mTo = to;
    mDate = date;
}

MailListDrag::MailListDrag( MailList mailList, TQWidget * parent, MailTextSource *src )
    : TQStoredDrag( MailListDrag::format(), parent ), _src(src)
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

bool MailListDrag::canDecode( TQMimeSource *e )
{
    return e->provides( MailListDrag::format() );
}

// Have to define before use
TQDataStream& operator<< ( TQDataStream &s, MailSummary &d )
{
    s << d.serialNumber();
    s << d.messageId();
    s << d.subject();
    s << d.from();
    s << d.to();
    s << d.date();
    return s;
}

TQDataStream& operator>> ( TQDataStream &s, MailSummary &d )
{
    Q_UINT32 serialNumber;
    TQString messageId, subject, from, to;
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

TQDataStream& operator<< ( TQDataStream &s, MailList &mailList )
{
    MailList::iterator it;
    for (it = mailList.begin(); it != mailList.end(); ++it) {
	MailSummary mailDrag = *it;
	s << mailDrag;
    }
    return s;
}

TQDataStream& operator>> ( TQDataStream &s, MailList &mailList )
{
    mailList.clear();
    MailSummary mailDrag;
    while (!s.atEnd()) {
	s >> mailDrag;
	mailList.append( mailDrag );
    }
    return s;
}

bool MailListDrag::decode( TQDropEvent* e, MailList& mailList )
{
    TQByteArray payload = e->encodedData( MailListDrag::format() );
    TQDataStream buffer( payload, IO_ReadOnly );
    if ( payload.size() ) {
	e->accept();
	buffer >> mailList;
	return TRUE;
    }
    return FALSE;
}

bool MailListDrag::decode( TQByteArray& payload, MailList& mailList )
{
    TQDataStream stream( payload, IO_ReadOnly );
    if ( payload.size() ) {
	stream >> mailList;
	return TRUE;
    }
    return FALSE;
}

bool MailListDrag::decode( TQDropEvent* e, TQByteArray &a )
{
    MailList mailList;
    if (decode( e, mailList )) {
	MailList::iterator it;
	TQBuffer buffer( a );
	buffer.open( IO_WriteOnly );
	TQDataStream stream( &buffer );
	for (it = mailList.begin(); it != mailList.end(); ++it) {
	    MailSummary mailDrag = *it;
	    stream << mailDrag.serialNumber();
	}
	buffer.close();
	return TRUE;
    }
    return FALSE;
}

void MailListDrag::setMailList( MailList mailList )
{
    TQByteArray array;
    TQBuffer buffer( array );
    buffer.open( IO_WriteOnly);
    TQDataStream stream( array, IO_WriteOnly );
    stream << mailList;
    buffer.close();
    setEncodedData( array );
}

const char *MailListDrag::format(int i) const
{
    if (_src) {
        if (i == 0) {
            return "message/rfc822";
        } else {
            return TQStoredDrag::format(i - 1);
        }
    }

    return TQStoredDrag::format(i);
}

bool MailListDrag::provides(const char *mimeType) const
{
    if (_src && TQCString(mimeType) == "message/rfc822") {
        return true;
    }

    return TQStoredDrag::provides(mimeType);
}

TQByteArray MailListDrag::encodedData(const char *mimeType) const
{
    if (TQCString(mimeType) != "message/rfc822") {
        return TQStoredDrag::encodedData(mimeType);
    }

    TQByteArray rc; 
    if (_src) {
        MailList ml;
        TQByteArray enc = TQStoredDrag::encodedData(format());
        decode(enc, ml);

        KProgressDialog *dlg = new KProgressDialog(0, 0, TQString::null, i18n("Retrieving and storing messages..."), true);
        dlg->setAllowCancel(true);
        dlg->progressBar()->setTotalSteps(ml.count());
        int i = 0;
        dlg->progressBar()->setValue(i);
        dlg->show();

        TQTextStream *ts = new TQTextStream(rc, IO_WriteOnly);
        for (MailList::ConstIterator it = ml.begin(); it != ml.end(); ++it) {
            MailSummary mailDrag = *it;
            *ts << _src->text(mailDrag.serialNumber());
            if (dlg->wasCancelled()) {
                break;
            }
            dlg->progressBar()->setValue(++i);
            kapp->eventLoop()->processEvents(TQEventLoop::ExcludeSocketNotifiers);
        }

        delete dlg;
        delete ts;
    }
    return rc;
}

