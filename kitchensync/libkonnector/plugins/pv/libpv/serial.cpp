/***************************************************************************
                          serial.cpp  -  description
                             -------------------
    begin                : Mon Nov 19 2001
    copyright            : (C) 2001 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// C++ includes
#include <iostream>
#include <fstream>
#include <iomanip>
// C includes
#include <stdio.h>			// Standard input/output definitions
#include <unistd.h>		// UNIX standard function definitions
#include <fcntl.h>			// File control definitions
#include <errno.h>			// Error number definitions
#include <sys/ioctl.h>
#include <sys/time.h>
// project includes
#include "serial.h"

using namespace std;

#ifdef DEBUG
#define debugout( out ) { std::cout << out << std::endl; }
#else
#define debugout( out ) { /* out */ }
#endif

/**
   * Constructor.
   */
CasioPV::Serial::Serial(){
	debugout( "BEGIN: Serial::Serial()" );

	m_fd = -1;				// Port not open
	m_lockfile = "";		// No lock file

	debugout( "END: Serial::Serial()" );
}

/**
   * Constructor.
   * @param port gives a char pointer to a string which contains the path to the device file
   */
CasioPV::Serial::Serial( const string& port ){
	debugout( "BEGIN: Serial::Serial( const char* )" );

	OpenPort( port );

	debugout( "END: Serial::Serial( const char* )" );
}

/**
   * Destructor.
   */
CasioPV::Serial::~Serial(){
	debugout( "BEGIN: Serial::~Serial()" );

	ClosePort();

	debugout( "END: Serial::~Serial()" );
}

/**
   * No descriptions
   */
void CasioPV::Serial::OpenPort( const string& port ){
	debugout( "BEGIN: Serial::OpenPort( const string& port )" );

	if ( m_fd != -1 ) throw SerialException( "Serial::OpenPort : Port already open", 3015 );

	lockDevice( port );

	m_fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if ( m_fd == -1) { // Could not open the port.
		unlockDevice();
		cerr << "Serial::OpenPort: Unable to open " << port << " - " << endl;
		throw SerialException( "Serial::OpenPort: Unable to open port ", 3001 );
	} else {
		// set the options for the port ???????????????????????????????
		fcntl(m_fd, F_SETFL, 0);
		// Get the current options for the port...
		tcgetattr(m_fd, &m_options);
		// Enable hardware flow control
//		m_options.c_cflag |= CRTSCTS;//(CLOCAL | CREAD | CRTSCTS);
		// Setting raw mode
		cfmakeraw(&m_options);
		// Set the new options for the port...
		tcsetattr(m_fd, TCSANOW, &m_options);
		// read non-blocking from port
		fcntl(m_fd, F_SETFL, FNDELAY);
		// set parity
		SetParity( NO_PARITY );
		// Set the baud rates to 9600...
		SetInputSpeed( 9600 );
		SetOutputSpeed( 9600 );
	}
//	debugout( "Port " << port << " successfully opened" );

	debugout( "END: Serial::OpenPort( const string& port )" );
}

/**
   * No descriptions
   */
void CasioPV::Serial::ClosePort(){
	debugout( "BEGIN: Serial::ClosePort()" );

	if ( m_fd != -1 ) {
		unlockDevice();
		close( m_fd );
		m_fd = -1;
	}

	debugout( "END: Serial::ClosePort(string port)" );
}

/**
   * No descriptions
   */
unsigned char CasioPV::Serial::ReadByte( int timeout ){										// parameter time out in msec

	if ( m_fd == -1 ) throw SerialException( "Serial::ReadByte : Port not open", 3005 );

	unsigned char received;
	timeval timestart, timenow;
	gettimeofday(&timestart, NULL);

	while ( !CheckForNextByte() ) {
		gettimeofday(&timenow, NULL);
		if ( (timeout > 0) &&
				(((timenow.tv_sec - timestart.tv_sec)*1000000 + (timenow.tv_usec - timestart.tv_usec)) > timeout*1000)) {
			throw SerialException( "Serial::ReadByte : timeout", 3002 );
		}
	}
	unsigned char* buffer = new unsigned char( 0x00 );
	if ( read(m_fd, buffer, 1) == -1 ) throw SerialException( "Serial::ReadByte : could not read next byte", 3003 );
	if ( *buffer == 0x18 ) throw SerialException( "Serial::ReadString : link packet", 3006 );
	received = *buffer;
	delete( buffer );
//	debugout( "received " << hex << (unsigned int)received << dec );

	return received;
}

/**
   * No descriptions
   */
string CasioPV::Serial::ReadString( unsigned int count, int timeout ){										// parameter time out in msec

	if ( m_fd == -1 ) throw SerialException( "Serial::ReadString : Port not open", 3005 );

	unsigned char* buffer = new unsigned char( 0x00 );
	string received = "";
	timeval timestart, timenow;

	for ( unsigned int i = 0; i<count; i++ ) {
		gettimeofday(&timestart, NULL);

		while ( !CheckForNextByte() ) {
			gettimeofday(&timenow, NULL);
			if ( (timeout > 0) &&
					(((timenow.tv_sec - timestart.tv_sec)*1000000 + (timenow.tv_usec - timestart.tv_usec)) > timeout*1000)) {
				throw SerialException( "Serial::ReadString : timeout", 3002 );
			}
		}
		if ( read(m_fd, buffer, 1) == -1 ) throw SerialException( "Serial::ReadString : could not read next byte", 3003);
		if ( *buffer == 0x18 ) throw SerialException( "Serial::ReadString : link packet", 3006 );
		received += *buffer;
	}
	delete( buffer );
//	debugout( "received " << received );

	return received;
}

/**
   * No descriptions
   */
void CasioPV::Serial::WriteByte( const unsigned char& byte ){

	if ( m_fd == -1 ) throw SerialException( "Serial::WriteByte : Port not open", 3005 );
	int n = write(m_fd, &byte, 1);
	if (n < 0) {
		cerr << "could not send " << hex << byte << endl;
		throw SerialException( "Serial::WriteByte : could not send", 3004 );
	}
//	debugout( hex << "byte " << byte << " sended" );

}

/**
   * No descriptions
   */
void CasioPV::Serial::WriteString( const string& bytes ){

	if ( m_fd == -1 ) throw SerialException( "Serial::WriteString : Port not open", 3005 );
	int n = write( m_fd, bytes.c_str(), bytes.length() );
	if (n < 0) {
		cerr << "could not send " << bytes << endl;
		throw SerialException( "Serial::WriteString : could not send", 3004 );
	}
//	debugout( hex << "bytes " << bytes << " sended" );

}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetOutputSpeed( int speed ){
	// Set the baud rates to ...
	/* B0, B50, B75, B110, B134, B150, B200, B300
		B600, B1200, B1800, B2400, B4800, B9600
		B19200, B38400, B57600, B115200, B230400
	*/
	if ( m_fd == -1 ) throw SerialException( "Serial::SetOutputSpeed : Port not open", 3005 );
	if ( tcgetattr(m_fd, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change output speed", 3007 );
	switch ( speed ) {
			case 1200			:	cfsetospeed(&m_options, B1200); break;
			case 2400			:	cfsetospeed(&m_options, B2400); break;
			case 4800			:	cfsetospeed(&m_options, B4800); break;
			case 9600			:	cfsetospeed(&m_options, B9600); break;
			case 19200		:	cfsetospeed(&m_options, B19200); break;
			case 38400		:	cfsetospeed(&m_options, B38400); break;
			case 57600		:	cfsetospeed(&m_options, B57600); break;
			case 115200	:	cfsetospeed(&m_options, B115200); break;
			case 230400	:	cfsetospeed(&m_options, B230400); break;
			default				:	return false;
	}
	// Set the new options for the port...
	if ( tcsetattr(m_fd, TCSANOW, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change output speed", 3007 );

	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetInputSpeed( int speed ){
	// Set the baud rates to ...
	/* B0, B50, B75, B110, B134, B150, B200, B300
		B600, B1200, B1800, B2400, B4800, B9600
		B19200, B38400, B57600, B115200, B230400
	*/
	if ( m_fd == -1 ) throw SerialException( "Serial::SetInputSpeed : Port not open", 3005 );
	if ( tcgetattr(m_fd, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change input speed", 3007 );
	switch ( speed ) {
			case 1200			:	cfsetispeed(&m_options, B1200); break;
			case 2400			:	cfsetispeed(&m_options, B2400); break;
			case 4800			:	cfsetispeed(&m_options, B4800); break;
			case 9600			:	cfsetispeed(&m_options, B9600); break;
			case 19200		:	cfsetispeed(&m_options, B19200); break;
			case 38400		:	cfsetispeed(&m_options, B38400); break;
			case 57600		:	cfsetispeed(&m_options, B57600); break;
			case 115200	:	cfsetispeed(&m_options, B115200); break;
			case 230400	:	cfsetispeed(&m_options, B230400); break;
			default				:	return false;
	}
	// Set the new options for the port...
	if ( tcsetattr(m_fd, TCSANOW, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change input speed", 3007 );

	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetDTR() {
	if ( m_fd == -1 ) throw SerialException( "Serial::SetDTR : Port not open", 3005 );
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::SetDTR : ioctl error", 3007 );
	status |= TIOCM_DTR;
	if ( ioctl(m_fd, TIOCMSET, &status) == -1 ) throw SerialException( "Serial::SetDTR : ioctl error", 3007 );

	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::ClearDTR() {
	if ( m_fd == -1 ) throw SerialException( "Serial::ClearDTR : Port not open", 3005 );
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::ClearDTR : ioctl error", 3007 );
	status &= ~TIOCM_DTR;
	if ( ioctl(m_fd, TIOCMSET, &status) == -1 ) throw SerialException( "Serial::ClearDTR : ioctl error", 3007 );
	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckDTR(){
	if ( m_fd == -1 ) throw SerialException( "Serial::CheckDTR : Port not open", 3005 );
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::CheckDTR : ioctl error", 3007 );
	if ( (status & TIOCM_DTR) == 2 ) {
       return true;
    }

	return false;
}

void CasioPV::Serial::SetParity( int parity ){

	if ( m_fd == -1 ) throw SerialException( "Serial::SetParity : Port not open", 3005 );
	if ( tcgetattr(m_fd, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change the parity", 3007 );
	switch ( parity ) {
		case NO_PARITY		 	:  // No parity (8N1)
		case SPACE_PARITY	:  // Space parity is the same as No parity (7S1)
				m_options.c_cflag &= ~PARENB;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS8;
				break;

		case EVEN_PARITY		:  // Even parity (7E1)
				m_options.c_cflag |= PARENB;
				m_options.c_cflag &= ~PARODD;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS7;
				break;

		case ODD_PARITY		:  // Odd parity (7O1)
				m_options.c_cflag |= PARENB;
				m_options.c_cflag |= PARODD;
				m_options.c_cflag &= ~CSTOPB;
				m_options.c_cflag &= ~CSIZE;
				m_options.c_cflag |= CS7;
				break;

	}
	if ( tcsetattr(m_fd, TCSANOW, &m_options) == -1 ) SerialException( "Serial::SetParity : could not change the parity", 3007 );

}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckForNextByte(){

	if ( m_fd == -1 ) throw SerialException( "Serial::CheckForByte : Port not open", 3005 );
	int bytes = 0;

	if ( ioctl(m_fd, FIONREAD, &bytes) == -1 ) throw SerialException( "Serial::CheckForNextByte : ioctl error", 3007 );
	if ( bytes ) return true;

	return false;
}


/**
   * No descriptions
   */
int CasioPV::Serial::GetState(){
    int state;
    if ( ioctl(m_fd, TIOCMGET, &state) == -1 ) throw SerialException( "Serial::GetState : ioctl error", 3007 );
	 return state;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetState( int state ){
    int tmp;
    if ( ioctl(m_fd, TIOCMGET, &tmp) == -1 ) throw SerialException( "Serial::SetState : ioctl error", 3007 );
    struct termios tmp2;
    tcgetattr(m_fd, &tmp2);
    tmp = state;
    if ( ioctl(m_fd, TIOCMSET, &tmp) == -1 ) throw SerialException( "Serial::SetState : ioctl error", 3007 );

    tcsetattr(m_fd, TCSANOW, &tmp2);
    if ( ioctl(m_fd, TIOCMSET, &tmp) == -1 ) throw SerialException( "Serial::SetState : ioctl error", 3007 );
    tcsetattr(m_fd, TCSANOW, &m_options);
    return true;
}

/**
   * No descriptions
   */
struct termios CasioPV::Serial::GetOptions(){
    tcgetattr(m_fd, &m_options);
    return m_options;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetOptions(struct termios comoptions){
    m_options = comoptions;
    tcsetattr(m_fd, TCSANOW, &m_options);
    return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckCTS(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::CheckCTS : ioctl error", 3007 );
	if ( (status & TIOCM_CTS) == 32 ) {
       return true;
    }

	return false;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::SetCAR(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::SetCAR : ioctl error", 3007 );
debugout( "set CAR status1: " << status );
	status |= TIOCM_CAR;
debugout( "set CAR status2: " << status );
	if ( ioctl(m_fd, TIOCMSET, &status) == -1 ) throw SerialException( "Serial::SetCAR : ioctl error", 3007 );
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::ClearCAR(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::ClearCAR : ioctl error", 3007 );
debugout( "clear CAR status1: " << status );
	status &= ~TIOCM_CAR;
debugout( "clear CAR status2: " << status );
	if ( ioctl(m_fd, TIOCMSET, &status) == -1 ) throw SerialException( "Serial::ClearCAR : ioctl error", 3007 );
	tcsetattr(m_fd, TCSANOW, &m_options);
	return true;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckCAR(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::CheckCAR : ioctl error", 3007 );
	if ( (status & TIOCM_CAR) == 64 ) {
		return true;
	}

	return false;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckDSR(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::CheckDSR : ioctl error", 3007 );
	if ( (status & TIOCM_DSR) == 256 ) {
       return true;
    }

	return false;
}

/**
   * No descriptions
   */
bool CasioPV::Serial::CheckRTS(){
	int status;

	if ( ioctl(m_fd, TIOCMGET, &status) == -1 ) throw SerialException( "Serial::CheckRTS : ioctl error", 3007 );
	if ( (status & TIOCM_RTS) == 4 ) {
       return true;
    }
	return false;
}

/**
   * No descriptions
   */
void CasioPV::Serial::lockDevice( const string& port ){
	debugout( "BEGIN: Serial::lockDevice( const string& port )" );

	if ( port.size() < 6 ) throw SerialException( "Serial::lockDevice: Device is invalid", 3016 );
	if (m_lockfile.size() != 0 ) throw SerialException( "Serial::lockDevice: Device already locked", 3017 );

	m_lockfile = "/var/lock/LCK.."; m_lockfile.append( port.c_str(), 5, port.size()-5 );

	// check for existing lockfile
        // FIXME
	ifstream ifile( m_lockfile.c_str()  );
	if ( ifile ) {
		ifile.close();
		cerr << "Serial::OpenPort: Device is locked" << endl;
		throw SerialException( "Serial::lockDevice: Device is locked", 3010 );
	}

	// create lockfile
	ofstream ofile( m_lockfile.c_str(), ios::in ); // maybe we need another umask entry !!, 644 );
	if ( !ofile ) {		// error: cannot create lockfile
		cerr << "Error while creating lockfile : " << m_lockfile << endl;
		throw SerialException( "Serial::lockDevice: Error while creating lockfile : "+m_lockfile, 3011 );
	}

	ofile << setw(10) << getpid();
	ofile.close();

	debugout( "END: Serial::lockDevice( const string& port )" );
}

/**
   * No descriptions
   */
void CasioPV::Serial::unlockDevice(){
	debugout( "BEGIN: Serial::unlockDevice()" );

	if ( unlink( m_lockfile.c_str() ) != 0 ) {
		// error: cannot delete lockfile
		cerr << "Error while deleting lockfile" << endl;
		throw SerialException( "Serial::unlockDevice: Error while deleting lockfile", 3012 );
	}
	m_lockfile = "";

	debugout( "END: Serial::unlockDevice()" );
}
