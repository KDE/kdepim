#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>

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
      return -1;
    }
  FD_ZERO(&set);
  FD_SET(serialPort, &set);
  select(serialPort + 1, &set, 0L, 0L, 0L);
  close(serialPort);
  return 0;
}
