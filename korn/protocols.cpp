/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "protocols.h"

#include "kio_proto.h"

#include "imap_proto.h"
#include "mbox_proto.h"
#include "pop3_proto.h"
#include "process_proto.h"
#include "imaps_proto.h"
#include "nntp_proto.h"
#include "pop3s_proto.h"
#include "qmail_proto.h"

#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>

QDict<KIO_Protocol>* Protocols::protocols = 0;

KIO_Protocol* Protocols::getProto( const QString& proto )
{
	if( !protocols )
		fillProtocols();
		
	return protocols->find( proto );
}

QStringList Protocols::getProtocols()
{
	QStringList output;
	
	if( !protocols )
		fillProtocols();
	
	QDictIterator<KIO_Protocol> it( *protocols );
	for( ; it.current(); ++it )
		output.append( it.currentKey() );

	output.sort();
	
	return output;
}
	
void Protocols::fillProtocols()
{
	protocols = new QDict< KIO_Protocol>;
	protocols->setAutoDelete( true );
	addProtocol( new Imap_Protocol );
	addProtocol( new Imaps_Protocol );
	addProtocol( new MBox_Protocol );
	addProtocol( new Pop3_Protocol );
	addProtocol( new Pop3s_Protocol );
	addProtocol( new Process_Protocol );
	addProtocol( new Nntp_Protocol );
	addProtocol( new QMail_Protocol );
}

void Protocols::addProtocol( KIO_Protocol* proto )
{
	protocols->insert( proto->configName(), proto );
}

