/* pilotListener.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a teeny-tiny program that opens the Pilot device and
** waits until something arrives on it. And then it exits. This
** is legacy code and should be removed as soon as we get a
** QSocketNotifier in the daemon.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef NDEBUG
static const char *id="$Id$";
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// pilotListener is such a small program -- and not even a real
// KDE app -- that it doesn't use kdDebug() &c. So the use of
// cerr is acceptable here.
//
//
int main(int argc, char* argv[])
{  
#ifndef NDEBUG
	int p = getpid();

	cerr << "*** " << id << endl;
	cerr << "*** " << argv[0] << " with pid=" << p << " started." << endl;
#endif

	if(argc < 2)
	{
		cerr << "*** Usage: " << argv[0] << " <pilotPort>" << endl;
		return 1;
	}

	int serialPort = open(argv[1], O_RDONLY | O_NONBLOCK);
	if(serialPort < 0)
	{
		cerr << "*** " << argv[1] << strerror(errno) << endl;
		return 1;
	}


	fd_set set;
	FD_ZERO(&set);
	FD_SET(serialPort, &set);

	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	int r = select(serialPort + 1, &set, 0L, 0L, &tv);
	if (r<0)
	{
		cerr << "*** " << argv[1] << strerror(errno) << endl;
		return 1;
	}
	if (r>0)
	{
		cerr << "*** Data arrived within 3 sec, draining." << endl;

		char buf[16];
		while ((r=read(serialPort,buf,16))>0)
		{
			cerr << "*** Drained " << r << " bytes." << endl;
		}
	}
	else
	{
		cerr << "*** Data drain timed out." << endl;
	}

	FD_ZERO(&set);
	FD_SET(serialPort, &set);
	r = select(serialPort + 1, &set, 0L, 0L, 0L);
	if (r<0)
	{
		cerr << "*** " << argv[1] << strerror(errno) << endl;
		return 1;
	}
	close(serialPort);

#ifndef NDEBUG
	cerr << "*** " << argv[0] << " with pid=" << p << " ended." << endl;
#endif

	return 0;
}


// $Log$
// Revision 1.10  2001/03/06 12:12:16  adridg
// Additional listener debugging
//
// Revision 1.9  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
