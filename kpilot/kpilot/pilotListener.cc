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
static const char *id="$Id$";

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>

// pilotListener is such a small program -- and not even a real
// KDE app -- that it doesn't use kdDebug() &c. So the use of
// cerr is acceptable here.
//
//
int main(int argc, char* argv[])
{  
  if(argc < 2)
    {
      cerr << "Usage: " << argv[0] << " <pilotPort>" << endl;
      return -1;
    }

  int serialPort = open(argv[1], O_RDONLY);
  fd_set set;

  if(serialPort == -1)
    {
      cerr << "Error opening " << argv[1] << endl;
	perror(argv[1]);
      return -1;
    }
  FD_ZERO(&set);
  FD_SET(serialPort, &set);
  select(serialPort + 1, &set, 0L, 0L, 0L);
  close(serialPort);
  return 0;
	/* NOTREACHED */
	(void) id;
}


// $Log:$
