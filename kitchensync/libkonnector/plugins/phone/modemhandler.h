/***************************************************************************
                          phoneconnection.h  -  description
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


#ifndef MODEMHANDLER_H
#define MODEMHANDLER_H

#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <qobject.h>

/**encapsulates all knowledge about the connection between the mobile phone and the computer
  *@author Matthias Welwarsky
  */

class QSocketNotifier;

class ModemHandler : public QObject  {
	Q_OBJECT

public:
	ModemHandler();
	virtual ~ModemHandler();

  	/** opens a connection to the mobile phone on the specified tty device.
	 *  return true on successful connection, false if the connection fails.
	 */
	bool openConnection(QString tty = 0, QString speed = 0);

  	/** return the last error message */
  	const QString& errorMessage() const;
  	/** returns a reference to the response last received from the mobile phone */
  	const QString& lastResponse() const;
  	/** send a command to the mobile phone. */
  	bool sendCommand(const QString&);
  	/** closes the connection to the mobile phone */
  	void closeConnection();
  	/** set the modem speed from a QString. */
  	void setModemSpeed(QString speed = 0);

signals: // Signals
  	/** emitted when a response from the modem was received */
  	void receivedResponse();
  	/** emits a complete response line from the mobile phone.
	 *  usually connected only to the message dispatcher. */
  	void responseReady(QString);

private: // Private methods
  	/** called from socketReady() when data from the modem is ready for to be read */
  	void newChar(unsigned char);
  	
  	/** stolen from kppp's "Modem"-Class */
  	bool writeLine(const char *);
  	bool opentty(const QString&);
  	bool closetty();
  	
private: // Private attributes
  	/** holds the latest response from the phone */
  	QString m_Response;

  	int modemfd;
  	speed_t modemSpeed;
  	struct termios initial_tty;
  	struct termios tty;
  	  	
  	QSocketNotifier *sn;
  	QString errmsg;

private slots: // Private slots
  	/** this is connected to the QSocketNotifier's signal "activated(int)" */
  	void socketReady(int);
};

#endif
