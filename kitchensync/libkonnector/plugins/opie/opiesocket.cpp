#include <qhostaddress.h>
#include <qtimer.h>
#include <koperations.h>
#include <kgenericfactory.h>
#include <qsocket.h>
#include <kdebug.h>

#include "opiesocket.h"

class OpieSocket::OpieSocketPrivate{
public:
    OpieSocketPrivate(){}
    QString pass;
    QString user;
    bool connected;
    QHostAddress src;
    QHostAddress dest;
    QSocket *socket;
    QTimer *timer;
    int mode;
    int getMode;
    enum Call{NOTSTARTED=0, HANDSHAKE=0, ABOOK, TODO, CALENDAR, TRANSACTIONS, FILES};
    enum Status {START=0, USER=1, PASS, CALL, NOOP, DONE , CONNECTED};
};

OpieSocket::OpieSocket(QObject *obj, const char *name )
    : QObject( obj, name )
{
    d = new OpieSocketPrivate;
    d->socket = 0;
    d->timer = 0;
    d->connected = false;
}
void OpieSocket::setUser(const QString &user )
{
    d->user = user;
}
void OpieSocket::setPassword(const QString &pass )
{
    d->pass = pass;
}
void OpieSocket::setSrcIP(const QHostAddress &src )
{
    d->src = src;
}

void OpieSocket::setDestIP(const QHostAddress &dest )
{
    d->dest = dest;
}
void OpieSocket::startUp() // start the connection
// and find out the the Homedir Path ;)
// start connecting
{
    kdDebug() << "OPieQCOPSocket" << endl;
    delete d->socket;
    d->socket = new QSocket(this, "OpieQCOPSocket" );
    connect( d->socket, SIGNAL(error(int) ), this,
	     SLOT(slotError(int) ) );
    connect( d->socket, SIGNAL(connected() ), this,
	     SLOT(slotConnected() ) );
    connect(d->socket, SIGNAL(connectionClosed() ), this,
	    SLOT(slotClosed() ) );
    connect(d->socket, SIGNAL(readyRead() ), this,
	    SLOT(process() ) );
    //connect(d->timer, SIGNAL(), this,
    //    SLOT(slotNOOP() ) );
    d->connected = false;
    d->socket->connectToHost(d->dest.toString(), 4243 );
}
bool OpieSocket::startSync()
{
    return false;
}
bool OpieSocket::isConnected()
{
    if( d->mode == d->CALL || d->mode == d->NOOP || d->mode == d->CONNECTED ){
	return true;
    }else{
	return false;
    }
}
QByteArray OpieSocket::retrFile(const QString &path )
{
    if( d->mode == d->NOOP ){
    //stop the timer 

    }
}
bool OpieSocket::insertFile( const QString &fileName )
{
// files
}
void OpieSocket::write(const QString &, const QByteArray & )
{

}
void OpieSocket::write(QPtrList<KSyncEntry> )
{

}
void OpieSocket::write(QValueList<KOperations> )
{

}
void OpieSocket::slotError(int error )
{

}
void OpieSocket::slotConnected()
{
    d->connected = true;
    delete d->timer;
    d->mode = d->START;
}
void OpieSocket::slotClosed()
{

}
void OpieSocket::process()
{
    while(d->socket->canReadLine() ){
	QTextStream stream( d->socket );
	QString line = d->socket->readLine();
	kdDebug() << line.stripWhiteSpace() << endl;
	switch( d->mode ){
	    case d->START: {
		if( line.left(3) != QString::fromLatin1("220") ){
		    // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		}else {
		    stream << "USER " << d->user << "\r\n";
		    d->mode = d->USER;
		}
		break;
	    }
	    case d->USER:{
		if(line.left(3) != QString::fromLatin1("331" ) ){
                // wrong user name
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		}else{
		    stream << "PASS " << d->pass << "\r\n";
		    d->mode = d->PASS;
		}
		break;
	    }
	    case d->PASS:{
		if(line.left(3) != QString::fromLatin1("230" ) ){
                // error
		    d->socket->close();
		    d->mode = d->DONE;
		    d->connected = false;
		}else{
                // ok start with the NOOP
                // start the Timer
		    kdDebug() << "start the NOOP" << endl;
		    d->mode = d->NOOP;
		    d->timer = new QTimer(this );
		    connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		    d->timer->start(10000 );
		}
		break;
	    }
	    case d->CALL:
		break;
	    case d->NOOP:
		//stream << "NOOP\r\n";
		d->mode = d->NOOP;
		d->timer = new QTimer(this );
		connect(d->timer, SIGNAL(timeout() ), this, SLOT(slotNOOP() ) );
		d->timer->start(10000 );
		break;
	};
    }
}
void OpieSocket::slotNOOP()
{
  QTextStream stream( d->socket );
  stream << "NOOP\r\n";
  kdDebug() << "NOOP" << endl;
  delete d->timer;
}



