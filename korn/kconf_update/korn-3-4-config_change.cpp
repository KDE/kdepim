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

#include <qstring.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qmap.h>
#include <qvaluelist.h>

#include <stdio.h>

QString decodeString( const QString &password )
{
	unsigned int i, val;
	unsigned int len = password.length();
	QString result="";
	for ( i=0; i < len; i++ )
	{
		val = password[i].latin1() - ' ';
		val = (255-' ') - val;
		result += (char)(val + ' ');
	}
	return result;
}

void printToprint( QTextStream &out, QMap<QString,QString> &to_printed, const QString type )
{
	out << "printToprint( " << type << " )" << endl;
	
	if( type == "mbox" )
	{
		out << "mailbox=" << to_printed[ "file" ] << endl;
	}
	else if( type == "qmail" )
	{
		out << "mailbox=" << to_printed[ "maildir" ] << endl;
	}
	else if( type == "pop3" )
	{
		out << "host=" << to_printed[ "host" ] << endl;
		out << "port=" << to_printed[ "port" ] << endl;
		out << "username=" << to_printed[ "user" ] << endl;
		if( to_printed[ "APOP" ] == "true" )
			out << "auth=APOP" << endl;
		else
			out << "auth=" << endl;
	}
	else if( type == "imap4" )
	{
		out << "host=" << to_printed[ "host" ] << endl;
		out << "port=" << to_printed[ "port" ] << endl;
		out << "username=" << to_printed[ "user" ] << endl;
		out << "mailbox=" << to_printed[ "mailbox" ] << endl;
	}
	else if( type == "nntp" )
	{
		out << "host=" << to_printed[ "host" ] << endl;
		out << "port=" << to_printed[ "port" ] << endl;
		out << "mailbox=" << to_printed[ "group" ] << endl;
	}
	else if( type == "process" )
	{
		out << "mailbox=" << to_printed[ "command" ] << endl;
	}
	else if( type == "kio" )
	{
		out << "host=" << to_printed[ "host" ] << endl;
		out << "port=" << to_printed[ "port" ] << endl;
		out << "username=" << to_printed[ "username" ] << endl;
		out << "mailbox=" << to_printed[ "mailbox" ] << endl;
		out << "password=" << decodeString( to_printed[ "password" ] ) << endl;
	}

	if( type == "pop3" || type == "imap4" )
	{
		out << "password=" << to_printed[ "pass" ] << endl;
		if( to_printed[ "pass" ].isEmpty()  )
			out << "savepassword=false" << endl;
		else
			out << "savepassword=true" << endl;
	}
	
	if( to_printed[ "resetcounter" ] != "-1" )
		out << "reset=" << to_printed[ "resetcounter" ] << endl;
	else
		out << "reset=0" << endl;
	out << "interval=" << to_printed[ "poll" ] << endl;
}

int main( int, char**  )
{
	QString line = QString::null;
	QString currentGroup1 = QString::null;
	QString currentGroup2 = QString::null;
	QString type = QString::null;
	QString password = QString::null;
	QRegExp interesting_group( "^\\[box-(\\d+)\\]" );
	QRegExp key_value( "^(\\w*)=(.*)$" );
	QValueList<QString> tobe_deleted;
	int numboxes = -1;
	bool isKey = false;
	
	QTextStream in( stdin, IO_ReadOnly );
	QTextStream out( stdout, IO_WriteOnly );

	in.setEncoding( QTextStream::UnicodeUTF8 );
        out.setEncoding( QTextStream::UnicodeUTF8 );

	QMap<QString,QString> mapping1;
	QValueList<QString> mapping2;
	QMap<QString,QString> to_printed;
	
	mapping1.insert( "caption", "name" );
	mapping1.insert( "onclick", "command" );
	mapping1.insert( "onnewmail", "newcommand" );
	mapping1.insert( "soundfile", "sound" );
	mapping1.insert( "passive_popup", "passivepopup" );
	mapping1.insert( "passive_data", "passivedata" );
	mapping1.insert( "reset", "reset" );
	mapping1.insert( "fgcolour", "normalfgcolour" );
	mapping1.insert( "bgcolour", "normalbgcolour" );
	mapping1.insert( "newmailfgcolour", "newfgcolour" );
	mapping1.insert( "newmailbgcolour", "newbgcolour" );
	mapping1.insert( "icon", "normalicon" );
	mapping1.insert( "newmailicon", "newicon" );
	
	mapping2.append( "file" );
	mapping2.append( "maildir" );
	mapping2.append( "host" );
	mapping2.append( "port" );
	mapping2.append( "user" );
	mapping2.append( "APOP" );
	mapping2.append( "mailbox" );
	mapping2.append( "group" );
	mapping2.append( "command" );
	mapping2.append( "protocol" );
	mapping2.append( "pass" );
	mapping2.append( "password" );
	mapping2.append( "resetcounter" );
	mapping2.append( "poll" );

	while( !in.atEnd() )
	{
		line = in.readLine();

		isKey = key_value.search( line ) >= 0;
		
		if( line.left( 1 ) == "[" )
		{
			if( !currentGroup1.isNull() )
			{
				out << currentGroup2 << endl;
				printToprint( out, to_printed, type );
			}
			
			currentGroup1 = QString::null;
		}
		
		if( interesting_group.search( line ) >= 0 )
		{
			if( numboxes > -1 && interesting_group.cap( 1 ).toInt() < numboxes )
			{
				currentGroup1 = QString( "[korn-%1]" ).arg( interesting_group.cap( 1 ) );
				currentGroup2 = QString( "[korn-%1-0]" ).arg( interesting_group.cap( 1 ) );
			}
			tobe_deleted.append( line );
			continue;
		}
		else if( isKey && key_value.cap( 1 ) == "numboxes" )
		{
			numboxes = key_value.cap( 2 ).toInt();
			continue;
		}
		else if( currentGroup1.isNull() || !isKey )
			continue;
		if( mapping1.contains( key_value.cap( 1 ) ) )
		{
			out << currentGroup1 << endl;
			out << mapping1[ key_value.cap( 1 ) ] << "=" << key_value.cap( 2 ) << endl;
			if( key_value.cap( 1 ) == "caption" )
			{
				out << currentGroup2 << endl;
				out << "name=" << key_value.cap( 2 ) << endl;
			}
		}
		else if( mapping2.contains( key_value.cap( 1 ) ) )
		{
			to_printed.insert( key_value.cap( 1 ), key_value.cap( 2 ) );
		}
		else if( key_value.cap( 1 ) == "type" && key_value.cap( 2 ) != "kio" )
		{
			out << currentGroup2 << endl;
			if( key_value.cap( 2 ) == "imap4" )
				out << "protocol=imap" << endl;
			else
				out << "protocol=" << key_value.cap( 2 ) << endl;
			type = key_value.cap( 2 );
			
		}
		else if( key_value.cap( 1 ) == "type" && key_value.cap( 2 ) == "kio" )
		{
			type = "kio";
		}
		else if( key_value.cap( 1 ) == "displaystyle" )
		{
			out << currentGroup1 << endl;
			if( key_value.cap( 2 ) == "2"  )
			{
				out << "hasnormalfgcolour=false" << endl;
				out << "hasnormalbgcolour=false" << endl;
				out << "hasnewfgcolour=false" << endl;
				out << "hasnewbgcolour=false" << endl;
				out << "hasnormalicon=true" << endl;
				out << "hasnormalanim=false" << endl;
				out << "hasnewicon=true" << endl;
				out << "hasnewanim=false" << endl;
			}
			else
			{
				out << "hasnormalfgcolour=true" << endl;
				out << "hasnormalbgcolour=true" << endl;
				out << "hasnewfgcolour=true" << endl;
				out << "hasnewbgcolour=true" << endl;
				out << "hasnormalicon=false" << endl;
				out << "hasnormalanim=false" << endl;
				out << "hasnewicon=false" << endl;
				out << "hasnewanim=false" << endl;
			}
		}
	}

	if( !currentGroup1.isNull() )
	{
		out << currentGroup2 << endl;
		printToprint( out, to_printed, type );
	}

	QValueList<QString>::Iterator it1     = tobe_deleted.begin();
	QValueList<QString>::Iterator it1_end = tobe_deleted.end();

	for( ; it1 != it1_end; ++it1 )
		out << "# DELETEGROUP " << *it1 << endl;

	return 0;
}

