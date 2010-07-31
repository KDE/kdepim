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
#include <tqbuffer.h>

class KTNEFMessage::MessagePrivate
{
public:
	MessagePrivate()
	{
		attachments_.setAutoDelete( true );
	}

	TQPtrList<KTNEFAttach> attachments_;
};

KTNEFMessage::KTNEFMessage()
{
	d = new MessagePrivate;
}

KTNEFMessage::~KTNEFMessage()
{
	delete d;
}

const TQPtrList<KTNEFAttach>& KTNEFMessage::attachmentList() const
{
	return d->attachments_;
}

KTNEFAttach* KTNEFMessage::attachment( const TQString& filename ) const
{
	TQPtrListIterator<KTNEFAttach> it( d->attachments_ );
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

TQString KTNEFMessage::rtfString()
{
	TQVariant prop = property( 0x1009 );
	if ( prop.isNull() || prop.type() != TQVariant::ByteArray)
		return TQString::null;
	else
	{
		TQByteArray rtf;
		TQBuffer input( prop.asByteArray() ), output( rtf );
		if ( input.open( IO_ReadOnly ) && output.open( IO_WriteOnly ) )
			lzfu_decompress( &input, &output );
		return TQString( rtf );
	}
}
