/*
    ktnefmessage.cpp

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

#include "ktnef/ktnefmessage.h"
#include "ktnef/ktnefattach.h"

#include "lzfu.h"
#include <qbuffer.h>
#include <QList>

class KTNEFMessage::MessagePrivate
{
public:
	MessagePrivate() {}
        ~MessagePrivate();

  void clearAttachments();

	QList<KTNEFAttach*> attachments_;
};

KTNEFMessage::MessagePrivate::~MessagePrivate()
{
  clearAttachments();
}

void KTNEFMessage::MessagePrivate::clearAttachments()
{
  while ( !attachments_.isEmpty() )
    delete attachments_.takeFirst();
}

KTNEFMessage::KTNEFMessage()
{
	d = new MessagePrivate;
}

KTNEFMessage::~KTNEFMessage()
{
	delete d;
}

const QList<KTNEFAttach*>& KTNEFMessage::attachmentList() const
{
	return d->attachments_;
}

KTNEFAttach* KTNEFMessage::attachment( const QString& filename ) const
{
  QList<KTNEFAttach*>::const_iterator it = d->attachments_.begin();
	for ( ; it != d->attachments_.end(); ++it )
		if ( (*it)->name() == filename )
			return *it;
	return 0;
}

void KTNEFMessage::addAttachment( KTNEFAttach *attach )
{
	d->attachments_.append( attach );
}

void KTNEFMessage::clearAttachments()
{
  d->clearAttachments();
}

QString KTNEFMessage::rtfString()
{
	QVariant prop = property( 0x1009 );
	if ( prop.isNull() || prop.type() != QVariant::ByteArray)
		return QString();
	else
	{
		QByteArray rtf;
                QByteArray propArray( prop.toByteArray() );
		QBuffer input( &propArray ), output( &rtf );
		if ( input.open( QIODevice::ReadOnly ) && output.open( QIODevice::WriteOnly ) )
			lzfu_decompress( &input, &output );
		return QString( rtf );
	}
}
