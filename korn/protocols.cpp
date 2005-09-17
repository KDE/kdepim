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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "protocols.h"

#include "kio_proto.h"

#include "imap_proto.h"
#include "mbox_proto.h"
#include "pop3_proto.h"
#include "process_proto.h"
//#include "imaps_proto.h"
#include "nntp_proto.h"
//#include "pop3s_proto.h"
#include "qmail_proto.h"
#include "dcop_proto.h"
#include "kmail_proto.h"

#include <QHash>
#include <qstring.h>
#include <qstringlist.h>

QHash< QString, Protocol* >* Protocols::protocols = 0;

const Protocol* Protocols::getProto( const QString& proto )
{
	if( !protocols )
		fillProtocols();

	if( !protocols->contains( proto ) )
		return 0;
		
	return protocols->value( proto );
}

const Protocol* Protocols::firstProtocol()
{
	return getProto( "mbox" );
}

QStringList Protocols::getProtocols() 
{
	QStringList output;
	
	if( !protocols )
		fillProtocols();
	
	QHash< QString, Protocol* >::const_iterator it;
	for( it = protocols->constBegin(); it != protocols->constEnd(); ++it )
		output.append( it.key() );

	output.sort();
	
	return output;
}
	
void Protocols::fillProtocols()
{
	protocols = new QHash< QString, Protocol* >;
	addProtocol( new Imap_Protocol );
	addProtocol( new MBox_Protocol );
	addProtocol( new Pop3_Protocol );
	addProtocol( new Process_Protocol );
	addProtocol( new Nntp_Protocol );
	addProtocol( new QMail_Protocol );
	addProtocol( new DCOP_Protocol );
	addProtocol( new KMail_Protocol );
}

void Protocols::addProtocol( Protocol* proto )
{
	protocols->insert( proto->configName(), proto );
}

