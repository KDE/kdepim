/*
    KMLOCfg

    A utility to configure the ELSA MicroLink(tm) Office modem.

    Copyright (C) 2000 Oliver Gantz <Oliver.Gantz@epost.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    ------
    ELSA and MicroLink are trademarks of ELSA AG, Aachen.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>

#include <qglobal.h>

#include <klocale.h>
#include <kdebug.h>

#include "modem.h"


#ifndef CSOH
#define CSOH  01
#endif

#ifndef CSTX
#define CSTX  02
#endif

#ifndef CEOT
#define CEOT  04
#endif

#ifndef CACK
#define CACK  06
#endif

#ifndef CNAK
#define CNAK 025
#endif

#ifndef CCAN
#define CCAN 030
#endif



Modem::Modem( KandyPrefs *kprefs, QObject *parent, const char *name ) :
  QObject(parent, name)
{
  mOpen = false;
  
  prefs = kprefs;

  timer = new QTimer( this, "modemtimer" );
  Q_CHECK_PTR( timer );
  connect( timer, SIGNAL( timeout() ), SLOT( timerDone() ) );

  init();
  xreset();
}


Modem::~Modem()
{
  close();
}


void Modem::setSpeed( int speed )
{
  switch ( speed ) {
    case 300:
      cspeed = B300;
      break;
    case 600:
      cspeed = B600;
      break;
    case 1200:
      cspeed = B1200;
      break;
    case 2400:
      cspeed = B2400;
      break;
    case 4800:
      cspeed = B4800;
      break;
    case 9600:
      cspeed = B9600;
      break;
    case 19200:
      cspeed = B19200;
      break;
    case 38400:
      cspeed = B38400;
      break;
    case 57600:
      cspeed = B57600;
      break;
    case 115200:
      cspeed = B115200;
      break;
    case 230400:
      cspeed = B230400;
      break;
    default:
#ifdef MODEM_DEBUG
      fprintf(stderr, "Modem: setSpeed(): fallback to default speed.\n");
#endif
      cspeed = B38400;
  }
}


void Modem::setData( int data )
{
  cflag &= ~CSIZE;

  switch ( data ) {
    case 5:
      cflag |= CS5;
      break;
    case 6:
      cflag |= CS6;
      break;
    case 7:
      cflag |= CS7;
      break;
    default:
      cflag |= CS8;
  }
}


void Modem::setParity( char parity )
{
  cflag &= ~( PARENB | PARODD );

  if ( parity == 'E' )
    cflag |= PARENB;
  else if ( parity == 'O' )
    cflag |= PARENB | PARODD;
}


void Modem::setStop( int stop )
{
  if (stop == 2)
    cflag |= CSTOPB;
  else
    cflag &= ~CSTOPB;
}


bool Modem::open()
{
  struct termios tty;


  close();

  if ( !lockDevice() )
    return false;

  const char *fdev = QFile::encodeName( (*prefs).serialDevice() ).data();
  if ( ( fd = ::open( fdev, O_RDWR | O_NOCTTY | O_NONBLOCK ) ) == -1 ) {
    emit errorMessage( i18n( "Unable to open device '%1'. "
                             "Please check that you have sufficient permissions." )
                             .arg( fdev ) );
    return false;
  }

  tcflush( fd, TCIOFLUSH );
  if ( tcgetattr( fd, &init_tty ) == -1 ) {
    emit errorMessage( i18n( "tcgetattr() failed." ) );
    ::close( fd );
    fd = 0;
    return false;
  }

  memset( &tty, 0, sizeof( tty ) );
  tty.c_iflag = IGNBRK | IGNPAR;
  tty.c_oflag = 0;
  tty.c_cflag = cflag;
  tty.c_lflag = 0;
  cfsetospeed( &tty, cspeed );
  cfsetispeed( &tty, cspeed );
  tcdrain( fd );

  if ( tcsetattr( fd, TCSANOW, &tty ) == -1 ) {
    emit errorMessage( i18n( "tcsetattr() failed." ) );
    ::close( fd );
    fd = 0;
    return false;
  }

  sn = new QSocketNotifier( fd, QSocketNotifier::Read, this,
                            "modemsocketnotifier" );
  Q_CHECK_PTR( sn );
  connect( sn, SIGNAL( activated( int ) ), SLOT( readChar( int ) ) );

  mOpen = true;

  return true;
}


void Modem::close()
{
  timer->stop();

  delete sn;
  sn = 0;

  if ( fd ) {
    tcflush( fd, TCIOFLUSH );
    tcsetattr( fd, TCSANOW, &init_tty );
    ::close( fd );
    fd = 0;
  }

  xreset();

  unlockDevice();

  mOpen = false;
}


void Modem::flush()
{
  if ( fd ) {
    tcflush( fd, TCIOFLUSH );
    bufpos = 0;
  }
}


bool Modem::lockDevice()
{
  ssize_t count;
  pid_t pid;
  int lfd;
  struct passwd *pw;
  QStringList pathList;
  QString fileName, content;


  if ( is_locked )
    return true;

  pathList = QStringList::split( "/", (*prefs).serialDevice() );
  fileName = (*prefs).lockDirectory() + "/LCK.." + pathList.last();

  if ( !access( QFile::encodeName( fileName ).data(), F_OK ) ) {
    char buf[256];


    if ( ( lfd = ::open( QFile::encodeName( fileName ), O_RDONLY ) ) < 0 ) {
      emit errorMessage( i18n( "Unable to open lock file '%1'.")
                               .arg( fileName ) );
      return false;
    }

    count = read( lfd, buf, 79 );

    if ( count < 0 ) {
       emit errorMessage( i18n( "Unable to read lock file '%1'.")
                                .arg( fileName ) );
       ::close( lfd );
       return false;
    }
    buf[ count ] = 0;
    ::close( lfd );

    count = sscanf( buf, "%d", &pid );
    if ( ( count != 1 ) || ( pid <= 0 ) ) {
       emit errorMessage( i18n( "Unable to get PID from file '%1'.")
                                .arg( fileName ) );
       return false;
    }

    if ( !kill( (pid_t) pid, 0 ) ) {
       emit errorMessage( i18n( "Process with PID %1, which is locking the device, is still running.")
                                .arg( pid ) );
       return false;
    }

    if ( errno != ESRCH ) {
      emit errorMessage( i18n( "Unable to emit signal to PID of existing lock file.") );
      return false;
    }
  }
	
  if ( ( lfd = creat( QFile::encodeName( fileName ).data(), 0644 ) ) == -1 ) {
    emit errorMessage( i18n( "Unable to create lock file '%1'. "
                             "Please check that you have sufficient permissions.")
                             .arg( fileName ) );
    return false;
  }

  pid = (int) getpid();
  pw = getpwuid( getuid() );
  content.sprintf( "%08d %s %s", pid, "kandy", pw->pw_name );
  write( lfd, QFile::encodeName( content ).data(), content.length() );
  ::close( lfd );

  is_locked = true;

  return true;
}


void Modem::unlockDevice()
{
  if ( is_locked ) {
    QStringList pathList = QStringList::split( "/", (*prefs).serialDevice() );

    QFile::remove( (*prefs).lockDirectory() + "/LCK.." + pathList.last() );
    is_locked = false;
  }
}


bool Modem::dsrOn()
{
  int flags;


  if ( !fd ) {
#ifdef MODEM_DEBUG
    fprintf( stderr, "Modem: dsrOn(): File not open.\n" );
#endif
    return false;
  }

  if ( ioctl( fd, TIOCMGET, &flags ) == -1 ) {
#ifdef MODEM_DEBUG
    fprintf( stderr, "Modem: dsrOn(): ioctl() failed.\n" );
#endif
    return false;
  }

  return ( flags & TIOCM_DSR ) != 0;
}


bool Modem::ctsOn()
{
  int flags;


  if ( !fd ) {
#ifdef MODEM_DEBUG
    fprintf( stderr, "Modem: ctsOn(): File not open.\n" );
#endif
    return false;
  }

  if ( ioctl( fd, TIOCMGET, &flags ) == -1) {
#ifdef MODEM_DEBUG
    fprintf( stderr, "Modem: ctsOn(): ioctl() failed.\n" );
#endif
    return false;
  }

  return ( flags & TIOCM_CTS ) != 0;
}


void Modem::writeChar( const char c )
{
  write( fd, (const void *) &c, 1 );
}


void Modem::writeLine( const char *line )
{
  kdDebug() << "Modem::writeLine(): " << line << endl;

  write( fd, (const void *) line, strlen( line ) );
  writeChar( '\r' );
}


void Modem::timerStart( int msec )
{
  timer->start( msec, true );
}


void Modem::receiveXModem( bool crc )
{
  disconnect( sn, 0, this, 0 );
  connect( sn, SIGNAL( activated( int ) ), SLOT( readXChar( int ) ) );

  xcrc = crc;

  if ( xcrc ) {
    writeChar( 'C' );
    xstate = 1;
    timerStart( 3000 );
  } else {
    writeChar( CNAK );
    xstate = 5;
    timerStart( 10000 );
  }

  xblock = 1;
}


void Modem::abortXModem()
{
  timer->stop();
  writeChar( CCAN );
  xreset();
  emit xmodemDone( false );
}


void Modem::timerDone()
{
#ifdef MODEM_DEBUG
  fprintf( stderr, "Modem: timeout, xstate = %d.\n", xstate );
#endif

  switch ( xstate ) {
    case  0:			/* non-XModem mode	*/
      emit timeout();
      break;

    case  1:			/* 1st 'C' sent		*/
    case  2:			/* 2nd 'C' sent		*/
    case  3:			/* 3rd 'C' sent		*/
      writeChar( 'C' );
      xstate++;
      timerStart( 1000 );	/* Should be 3000 in original XModem	*/
      break;

    case  4:			/* 4th 'C' sent		*/
      xcrc = false;

    case  5:			/* 1st <NAK> sent	*/
    case  6:			/* 2nd <NAK> sent	*/
    case  7:			/* 3rd <NAK> sent	*/
    case  8:			/* 4th <NAK> sent	*/
    case  9:			/* 5th <NAK> sent	*/
      writeChar( CNAK );
      xstate++;
      timerStart( 1000 );	/* Should be 10000 in original XModem	*/
      break;

    case 10:			/* 6th <NAK> sent	*/
      xreset();
      emit xmodemDone( false );
      break;

    default:			/* pending XModem block	*/
      writeChar( CNAK );
      xstate = 5;
      timerStart( 1000 );	/* Should be 10000 in original XModem	*/
	}
}


void Modem::readChar( int )
{
  uchar c;


  while ( read( fd, (void *) &c, 1 ) == 1 ) {
    if ( c == '\n' ) {
      buffer[ bufpos ] = 0;
      bufpos = 0;
      emit gotLine( (const char *) buffer );
      break;
    } else
    if ( ( bufpos < 1000 ) && ( c != '\r' ) )
      buffer[ bufpos++ ] = c;
  }
}


void Modem::readXChar( int )
{
  uchar c;
  static uchar crc_hi, block, cblock;


  while ( read( fd, (void *) &c, 1 ) == 1 ) {
    switch ( xstate ) {
      case  1:	/* 1st 'C' sent 	*/
      case  2:	/* 2nd 'C' sent		*/
      case  3:	/* 3rd 'C' sent		*/
      case  4:	/* 4th 'C' sent		*/
      case  5:	/* 1st <NAK> sent	*/
      case  6:	/* 2nd <NAK> sent	*/
      case  7:	/* 3rd <NAK> sent	*/
      case  8:	/* 4th <NAK> sent	*/
      case  9:	/* 5th <NAK> sent	*/
      case 10:	/* 6th <NAK> sent	*/
	if ( c == CSOH ) {
	  timerStart( 1000 );
	  xsize = 128;
	  xstate = 11;
	} else
	if ( c == CSTX ) {
	  timerStart( 1000 );
	  xsize = 1024;
	  xstate = 11;
        } else
	if ( c == CEOT ) {
	  timer->stop();
	  writeChar( CACK );
	  xreset();
	  emit xmodemDone( true );
	} else
	  timerStart( 1000 );
	break;

      case 11:	/* <SOH> or <STX> received	 */
	timerStart( 1000 );
	block = c;
	xstate++;
	break;

      case 12:	/* block number received	*/
	timerStart( 1000 );
	cblock = c;
	xstate++;
	bufpos = 0;
	break;

      case 13:	/* complement block number received	*/
	timerStart( 1000 );
	buffer[ bufpos++ ] = c;
	if ( bufpos == xsize ) {
	  bufpos = 0;
	  xstate++;
	  if ( !xcrc )
	    xstate++;
	}
	break;

      case 14:	/* data block received	*/
	timerStart( 1000 );
	crc_hi = c;
	xstate++;
	break;

      case 15:	/* crc high-byte received	*/
	timerStart( 10000 );
	xstate = 4;
	if ( (uchar) ( block ^ cblock ) != 0xff ) {
	  writeChar( CNAK );
	  break;
	}
	if ( block+1 == xblock ) {
	  writeChar( CACK );
	  break;
	}
	if ( block != xblock ) {
	  timer->stop();
	  writeChar( CCAN );
	  xreset();
	  emit xmodemDone( false );
	  break;
	}
	if ( xcrc ) {
	  if ( ( (ushort) crc_hi << 8 | (ushort) c ) != calcCRC() ) {
	    writeChar( CNAK );
	    break;
	  }
	} else {
	  if ( c != calcChecksum() ) {
	    writeChar( CNAK );
	    break;
	  }
	}
	writeChar( CACK );
	xblock++;
	emit gotXBlock( buffer, xsize );
	break;

      default:
	break;
    }
  }
}


void Modem::init()
{
  is_locked = false;

  fd = 0;
  sn = 0;

  cspeed = B38400;

  // No flow control
  cflag = CS8 | CREAD | CLOCAL;
  // cflag = CS8 | CREAD | CLOCAL | CRTSCTS;

  bufpos = 0;
}


void Modem::xreset()
{
  bufpos = 0;

  xstate = 0;
  xcrc = false;
  xblock = 0;
  xsize = 0;

  if ( sn ) {
    disconnect( sn, 0, this, 0 );
    connect( sn, SIGNAL( activated( int ) ), SLOT( readChar( int ) ) );
  }
}


uchar Modem::calcChecksum()
{
  int i;
  uchar c = 0;


  for ( i = 0; i < xsize; i++ )
    c += buffer[ i ];

  return c;
}


ushort Modem::calcCRC()
{
  int i, j;
  ushort c = 0;

	
  for ( i = 0; i < xsize; i++ ) {
    c ^= (ushort) buffer[ i ] << 8;
    for ( j = 0; j < 8; j++ )
      if ( c & 0x8000 )
        c = c << 1 ^ 0x1021;
      else
	c <<= 1;
  }

  return c;
}

#include "modem.moc"
