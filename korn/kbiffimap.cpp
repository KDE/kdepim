/*
 * kbiffimap.cpp -- Implementation of class KBiffImap.
 * Author: Kurt Granroth (granroth@kde.org)
 * Version: $Id$
 */

#include "utils.h"
#include "kbiffimap.h"

#include <qregexp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

KBiffSocket::KBiffSocket()
	: _messages( -1 )
{
}

KBiffSocket::~KBiffSocket()
{
	close();
}

void KBiffSocket::close()
{
	::close(_socket);
}

bool KBiffSocket::connect(const QString & _host, unsigned int port)
{
	// test to see if _host is empty
	if (_host.isNull()) return false;
	 
	QCString host(_host.ascii());
	sockaddr_in  sin;
	hostent     *hent; 
	int addr;


	// get the socket
	_socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

	// start setting up the socket info
	memset( ( char * )&sin, 0, sizeof( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_port   = htons( port );

	// get the address
	if( ( addr = inet_addr( host ) ) == -1 )
	{
		// get the address by host name
		if( ( hent = gethostbyname( host ) ) == 0 )
			return false;

		memcpy( static_cast<void *>(&sin.sin_addr), *(hent->h_addr_list), hent->h_length );
	}
	else
		// get the address by IP
		memcpy( static_cast<void *>(&sin.sin_addr), static_cast<void *>(&addr), sizeof( addr ) );

	// the socket is correctly setup.  now connect
	if( ::connect( _socket, reinterpret_cast<sockaddr *>(&sin), sizeof( sockaddr_in ) ) == -1 )
		return false;

	// we're connected!  see if the connection is good
	QString line( readLine() );
	if( ( line.find(fu("OK")) == -1 ) && ( line.find(fu("PREAUTH")) == -1) )
		return false;

	// everything is swell
	return true;
}

int KBiffSocket::writeLine(const QString& line)
{
	int bytes;

  // 3rd param was line.length() - 1 !
	if( (bytes = ::write(_socket, line.ascii(), line.length()) ) <= 0 )
		close();

	return bytes;
}

QString KBiffSocket::readLine()
{
	QString response;
	char buffer;

	while( ( ::read( _socket, &buffer, 1 ) > 0 ) && ( buffer != '\n' ) )
		response += buffer;

	return response;
}

bool KBiffImap::command( const QString& line, unsigned int seq )
{
	int len, match;

	if( writeLine( line ) <= 0 )
		return false;

	QString ok(fu("%1 OK"));
  QString response;
	ok = ok.arg(seq);

	response = readLine();
	while (!response.isEmpty())
	{
		// if the response is either good or bad, then return
		if( response.find( ok ) > -1 )
			return true;
		if( response.find(fu("BAD")) > -1 )
			return false;
		if( response.find(fu("NO ")) > -1 )
			return false;

		// check for new mail
		QRegExp recent_re(fu("UNSEEN [0-9]*"));
		if( ( match = recent_re.search( response ) ) > -1 )
                {
                        len = recent_re.matchedLength();
			_messages = response.mid( match + 7, len - 7 ).toInt();
                }
	
    response = readLine();
	}

	return false;
}

QString KBiffImap::mungeUser(const QString& old_user)
{
	if( old_user.contains(' ') > 0 )
	{
		QString new_user( old_user );

		if( new_user.left( 1 ) != fu("\"") )
			new_user.prepend(fu("\""));
		if( new_user.right( 1 ) != fu("\"") )
			new_user.append(fu("\""));

		return new_user;
	}
	else
		return old_user;
}
