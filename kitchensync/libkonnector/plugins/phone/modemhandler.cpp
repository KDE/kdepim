/***************************************************************************
                          phoneconnection.cpp  -  description
                             -------------------
    begin                : Wed Jan 17 2001
    copyright            : (C) 2001 by Matthias Welwarsky
    email                : matze@stud.fbi.fh-darmstadt.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>

#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>

#include <qsocketnotifier.h>

#include "modemhandler.h"

ModemHandler::ModemHandler() {
}

ModemHandler::~ModemHandler() {
}

/** opens a connection to the mobile phone on the specified tty device.
 *  returns true on successful connection, false if the connection fails.
 */
bool ModemHandler::openConnection(QString tty, const QString speed) {
	
	setModemSpeed(speed);
	
	if (tty == 0) {
		KConfig *config = kapp->config();
		config->setGroup("Communication");
		tty = config->readEntry("Device", "/dev/modem");
	}

	kdDebug() << "opening device " << tty << endl;
		
	if (!opentty(tty)) {
		return false;	
	}
	// return success
	return true;
}

/** return the last error message */
const QString& ModemHandler::errorMessage() const {
	return errmsg;
}

/** called by the modem object if a new character from the modem arrives. */
void ModemHandler::newChar(unsigned char data){

	switch (data) {
	case '\r':
		if (!m_Response.isEmpty()) {
			emit responseReady(m_Response);
		    m_Response = QString::null;
		}
		break;
		
	case '\n': // swallow
		break;

	default:
		m_Response += data;
		if (m_Response == "> ") {
			emit responseReady(m_Response);
			m_Response = QString::null;
		}
		break;
	}
}

/** returns a reference to the response last received from the mobile phone (deprecated) */
const QString& ModemHandler::lastResponse() const{
	return m_Response;
}

/** send a command to the mobile phone. The String must not be terminated by a linefeed */
bool ModemHandler::sendCommand(const QString& command){
	writeLine(command);
	return true;
}
/** closes the connection to the mobile phone */
void ModemHandler::closeConnection(){
	closetty();
}

static sigjmp_buf jmp_buffer;
void alarm_handler(int) {
  	siglongjmp(jmp_buffer, 1);
}

/** lowlevel modem communication */
bool ModemHandler::opentty(const QString& ttyDevice) {

    if (sigsetjmp(jmp_buffer, 1) == 0) {
      	// set alarm in case open() hangs, might happen with IrDA
      	signal(SIGALRM, alarm_handler);
      	alarm(2);

		if ((modemfd = ::open(ttyDevice, O_RDWR)) < 0) {
			errmsg = i18n("cannot open the modem device");
   			alarm(0);
   			signal(SIGALRM, SIG_IGN);
			return false;
		}
		alarm(0);
		signal(SIGALRM, SIG_IGN);
	} else {
		errmsg = i18n("Sorry, the phone does not respond.");
		return false;
	}

  	tcdrain (modemfd);
  	tcflush (modemfd, TCIOFLUSH);

	if (tcgetattr(modemfd, &tty) < 0) {
		errmsg = i18n("Sorry, the phone is busy.");
       	::close(modemfd);
       	modemfd = -1;

       	return false;
 	}

   	memset(&initial_tty,'\0',sizeof(initial_tty));

   	initial_tty = tty;

   	tty.c_cc[VMIN] = 0; // nonblocking
   	tty.c_cc[VTIME] = 0;
   	tty.c_oflag = 0;
   	tty.c_lflag = 0;

   	tty.c_cflag &= ~(CSIZE | CSTOPB | PARENB);
   	tty.c_cflag |= CS8 | CREAD;
   	tty.c_cflag |= CLOCAL;                   // ignore modem status lines
   	tty.c_iflag = IGNBRK | IGNPAR /* | ISTRIP */ ;
   	tty.c_lflag &= ~ICANON;                  // non-canonical mode
   	tty.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHOKE);
   	
   	// no handshake
   	tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF);

   	cfsetospeed(&tty, modemSpeed);
   	cfsetispeed(&tty, modemSpeed);

   	tcdrain(modemfd);	

   	if(tcsetattr(modemfd, TCSANOW, &tty) < 0){
     	errmsg = i18n("Sorry, the modem is busy.");
     	::close(modemfd);
     	modemfd=-1;
     	return false;
   	}
   	errmsg = i18n("Modem Ready.");
   	
   	sn = new QSocketNotifier(modemfd, QSocketNotifier::Read, this, "ttyNotifier");
   	connect(sn, SIGNAL(activated(int)), this, SLOT(socketReady(int)));
   	sn->setEnabled(true);
   	return true;
}


bool ModemHandler::closetty() {
  	if(modemfd >=0 ) {
  		sn->setEnabled(false);
    	delete sn;
    	
    	/* discard data not read or transmitted */
    	tcflush(modemfd, TCIOFLUSH);

    	// try to restore the previous tty settings
    	// this usually works, and if not, nothing can be done either
    	tcsetattr(modemfd, TCSANOW, &initial_tty);
    	::close(modemfd);
    	modemfd = -1;
	}
	return true;
}

bool ModemHandler::writeLine(const char *buf) {
  	int len = strlen(buf);
  	const char* bp = buf;
    int l = len;
  	
  	while (l) {
    	int wr = write(modemfd, bp, l);
    	
    	if(wr < 0)
      		return false;
    	
    	l -= wr;
    	bp += wr;
  	}
  	return true;
}

/** No descriptions */
void ModemHandler::socketReady(int){
  	char buffer[200];
  	int len;

  	// read data in chunks of up to 200 bytes
  	if((len = ::read(modemfd, buffer, 200)) > 0) {
    	// split buffer into single characters for further processing
    	for(int i = 0; i < len; i++)
      		newChar(buffer[i]);
  	}
}
/** set the modem speed from a QString. */
void ModemHandler::setModemSpeed(QString speed) {

	if (speed == 0) {
		KConfig *config = kapp->config();
		config->setGroup("Communication");
		speed = config->readEntry("Bitrate", "115200");
	}
	
	kdDebug() << "Setting ModemSpeed to "<< speed << endl;
	
	if (speed == "9600")
		modemSpeed = B9600;
	else if (speed == "19200")
		modemSpeed = B19200;
	else if (speed == "38400")
		modemSpeed = B38400;
	else if (speed == "57600")
		modemSpeed = B57600;
	else
		modemSpeed = B115200;
}
#include "modemhandler.moc"
