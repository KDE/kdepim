// pilotListener.cc
//
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
static const char *id="$Id$";

#include "options.h"

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
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
}
