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

#ifndef MODEM_H
#define MODEM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <termios.h>

#include <qobject.h>
#include <qstring.h>
#include <qtimer.h>
#include <qsocketnotifier.h>

#include "kandyprefs.h"




class Modem : public QObject
{
	Q_OBJECT
public:
	Modem(KandyPrefs *kprefs, QObject *parent = 0, const char *name = 0);
	virtual ~Modem();

	void setSpeed(int speed);
	void setData(int data);
	void setParity(char parity);
	void setStop(int stop);

	bool open();
	void close();

        bool isOpen() { return mOpen; }

	void flush();

	bool lockDevice();
	void unlockDevice();

	bool dsrOn();
	bool ctsOn();

	void writeChar(const char c);
	void writeLine(const char *line);

	void timerStart(int msec);

	void receiveXModem(bool crc);
	void abortXModem();

private slots:
	void timerDone();

	void readChar(int);
	void readXChar(int);

private:
  bool mOpen;

	void init();
	void xreset();

	uchar calcChecksum();
	ushort calcCRC();

	bool is_locked;
	struct termios init_tty;

	speed_t cspeed;
	tcflag_t cflag;

	int fd;
	QTimer *timer;
	QSocketNotifier *sn;

	uchar buffer[1024];
	int bufpos;

	int xstate;
	bool xcrc;
	uchar xblock;
	int xsize;

        KandyPrefs *prefs;

signals:
	void gotLine(const char *);
	void gotXBlock(const uchar *, int);
	void xmodemDone(bool);
	void timeout();

    void errorMessage( const QString & );
};


#endif // MODEM_H
