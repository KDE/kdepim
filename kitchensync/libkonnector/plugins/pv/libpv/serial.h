/***************************************************************************
                          serial.h  -  description
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

#ifndef SERIAL_H
#define SERIAL_H

// C includes
#include <stdlib.h>
#include <termios.h>	/* POSIX terminal control definitions */

//project includes
#include <serialexception.h>

#define NO_PARITY				1			// 8N1
#define EVEN_PARITY			2			// 7E1
#define ODD_PARITY			3			// 7O1
#define SPACE_PARITY		4			// 7S1

namespace CasioPV {

struct strpointer {
	unsigned int length;
	unsigned char* bytes;
};

/**This is a class to control a serial port. It can write, read and set the speed of a serial port.
  *@author Selzer Michael
  */

class Serial {
	public:
		/**
		   * Constructor.
		   */
		Serial();
		/**
		   * Constructor.
		   * @param port gives a char pointer to a string which contains the path to the device file
		   */
		Serial(const string& port);
		/**
		   * Destructor.
		   */
		~Serial();

		/**
		   * No descriptions
		   */
		void OpenPort( const string& port );

		/**
		   * No descriptions
		   */
		void ClosePort();

		/**
		   * No descriptions
		   */
		unsigned char ReadByte( int timeout=0 );

		/**
		   * No descriptions
		   */
		string ReadString( unsigned int count, int timeout=0 );

		/**
		   * No descriptions
		   */
		void WriteByte( const unsigned char& byte );

		/**
		   * No descriptions
		   */
		void WriteString( const string& bytes );

		/**
		   * No descriptions
		   */
		bool SetOutputSpeed( int speed );

		/**
		   * No descriptions
		   */
		bool SetInputSpeed( int speed );

		/**
		   * No descriptions
		   */
		void SetParity( int parity );

		/**
		   * No descriptions
		   */
		bool SetDTR();

		/**
		   * No descriptions
		   */
		bool ClearDTR();

		/**
		   * No descriptions
		   */
		bool CheckDTR();

		/**
		   * No descriptions
		   */
		bool CheckForNextByte();

		/**
		   * No descriptions
		   */
		bool SetState(int state);

		/**
		   * No descriptions
		   */
		int GetState();

		/**
		   * No descriptions
		   */
		bool SetOptions(struct termios comoptions);

		/**
		   * No descriptions
		   */
		struct termios GetOptions();

		/**
		   * No descriptions
		   */
		bool CheckDSR();

		/**
		   * No descriptions
		   */
		bool CheckCAR();

		/**
		   * No descriptions
		   */
		bool ClearCAR();

		/**
		   * No descriptions
		   */
		bool SetCAR();

		/**
		   * No descriptions
		   */
		bool CheckCTS();

		/**
		   * No descriptions
		   */
		bool CheckRTS();

		/**
		   * No descriptions
		   */
		bool ClearRTS();

		/**
		   * No descriptions
		   */
		bool SetRTS();

	private:
		int m_fd;
		string m_lockfile;
		termios m_options;	// Struct for serial parameters
		unsigned char m_received;

		/**
		   * No descriptions
		   */
		void lockDevice( const string& port );

		/**
		   * No descriptions
		   */
		void unlockDevice();		

};

}; // namespace CasioPV

#endif
