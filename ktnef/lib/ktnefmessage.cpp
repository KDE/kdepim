/*
    ktnefmessage.cpp

    Copyright (C) 2002 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "ktnef/ktnefmessage.h"
#include "ktnef/ktnefattach.h"

#include "lzfu.h"
#include <qbuffer.h>

class KTNEFMessage::MessagePrivate
{
public:
	MessagePrivate()
	{
		attachments_.setAutoDelete( true );
	}

	QPtrList<KTNEFAttach> attachments_;
};

KTNEFMessage::KTNEFMessage()
{
	d = new MessagePrivate;
}

KTNEFMessage::~KTNEFMessage()
{
	delete d;
}

const QPtrList<KTNEFAttach>& KTNEFMessage::attachmentList() const
{
	return d->attachments_;
}

KTNEFAttach* KTNEFMessage::attachment( const QString& filename ) const
{
	QPtrListIterator<KTNEFAttach> it( d->attachments_ );
	for ( ; it.current(); ++it )
		if ( it.current()->name() == filename )
			return it.current();
	return 0;
}

void KTNEFMessage::addAttachment( KTNEFAttach *attach )
{
	d->attachments_.append( attach );
}

void KTNEFMessage::clearAttachments()
{
	d->attachments_.clear();
}

QString KTNEFMessage::rtfString()
{
	QVariant prop = property( 0x1009 );
	if ( prop.isNull() || prop.type() != QVariant::ByteArray)
		return QString::null;
	else
	{
		QByteArray rtf;
		QBuffer input( prop.asByteArray() ), output( rtf );
		if ( input.open( IO_ReadOnly ) && output.open( IO_WriteOnly ) )
			lzfu_decompress( &input, &output );
		return QString( rtf );
	}
}
