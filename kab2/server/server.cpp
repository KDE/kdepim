#include <iostream>
#include <qcstring.h>
#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h> 

#include <KabAddressBook.h>
#include <KabEntity.h>
#include <KabGroup.h>
#include <qstrlist.h>
#include <qdatastream.h>

int version = 1; // SET THIS WHEN THE PROTOCOL IS CHANGED !!
int PORT = 6566;

  inline Q_UINT32
decodeToInt(char * s)
{ return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3]; }

  char *
fourOctets(Q_UINT32 i)
{
  char * s = new char[4];

  s[0] = (i >> 24)  & 0x000000ff;
  s[1] = (i >> 16)  & 0x000000ff;
  s[2] = (i >> 8)   & 0x000000ff;
  s[3] = i          & 0x000000ff;

  return s;
}


const QCString welcomeMessage = "KAB " + QCString().setNum(version) + "\n"; 

KAB::AddressBook * ab;

void doReplace  (int fd, const QByteArray & data);
void doErase    (int fd, const QByteArray & data);
void doAdd      (int fd, const QByteArray & data);
void doFind     (int fd, const QByteArray & data);
void doSearch   (int fd, const QByteArray & data);
void doList     (int fd);

  bool
processCommand(int fd)
{
  int nrecv;
  
  char commandType;

  cerr << "Server: Waiting for command" << endl;
  // First find out what type of request we're doing.
  nrecv = ::read(fd, &commandType, (size_t)1);
  
  if (nrecv != 1) {
    cerr << "Server: Connection died" << endl;
    return false;
  }

  cerr << "Server: Command: " << commandType << endl;

  if (commandType == 'q') {
    cerr << "Server: Quit requested" << endl;
    return false;
  }
  
  QByteArray s;
  char * buf = 0;
  Q_UINT32 totalOctets = 0;
  
  if (commandType != 'l') {

    char total[4];

    nrecv = ::read(fd, total, (size_t)4);

    if (nrecv != 4) {
      cerr << "Server: Couldn't get total number of octets to read" << endl;
      return false;
    }

    Q_UINT32 totalOctets = decodeToInt(total);

    if (totalOctets == 0) {
      cerr << "Server: Total number of octets to read is 0" << endl;
      return false;
    }

    char * buf = new char[totalOctets];

    nrecv = ::read(fd, buf, (size_t)totalOctets);

    if (nrecv != (int)totalOctets) {
      cerr << "Server: Couldn't read " << totalOctets << " octets" << endl;
      delete [] buf;
      return false;
    }


    s.setRawData(buf, totalOctets);
  }

  switch (commandType) {
    case 'l': doList    (fd);     break;
    case 'a': doAdd     (fd, s);  break;
    case 'e': doErase   (fd, s);  break;
    case 'r': doReplace (fd, s);  break;
    case 'f': doFind    (fd, s);  break;
    case 's': doSearch  (fd, s);  break;
    default:                      break;
  }

  if (commandType != 'l') {
    s.resetRawData(buf, totalOctets);
    delete [] buf;
  }
  
  cerr << "Server: Done command" << endl;
  return true;
}

  int
createSocket()
{
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) perror("socket");
  return fd;
}

  int
bindToSocket(int fd)
{
  struct sockaddr_in local_addr;
  local_addr.sin_family = AF_INET;

  int port = PORT; // "AB"
  local_addr.sin_port = htons(port);
  local_addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(local_addr.sin_zero), 0, 8);

  int size_sockaddr = sizeof(struct sockaddr);

  int i = ::bind(fd, (struct sockaddr *)&local_addr, size_sockaddr);
  if (i == -1) perror("bind");
  return i;
}

  int
listenToSocket(int fd)
{
  int i = ::listen(fd, 10); // Allow 10 backed up connections.
  if (i == -1) perror("listen");
  return i;
}
    
  int
acceptOnSocket(int fd)
{
  size_t sin_size = sizeof(struct sockaddr_in);
  struct sockaddr_in remoteAddr;
  int i = ::accept(fd, (struct sockaddr *)&remoteAddr, &sin_size);
  if (i == -1) perror("accept");
  return i;
}

  int
setupConnection()
{
  int server_fd = createSocket();
  
  if (server_fd == -1) {
    cerr << "Server: Couldn't create server socket" << endl;
    exit(1);
  }
  
  int i;
  
  i = bindToSocket(server_fd);
  
  if (i == -1) {
    cerr << "Server: Couldn't bind to server socket" << endl;
    exit(1);
  }
  
  i = listenToSocket(server_fd);
  
  if (i == -1) {
    cerr << "Server: Couldn't listen on server socket" << endl;
    exit(1);
  }

  return server_fd;
}

  int
main(int argc, char **argv)
{
  if (argc != 3) {
    cerr << "usage: " << argv[0] << " <format> <location>" << endl;
    exit(1);
  }

  int server_fd = setupConnection();

  cerr << "KAB TCP server listening on port " << PORT << endl;

  QString format(argv[1]);
  QString location(argv[2]);

  ab = KAB::AddressBook::create("localhost", format, location);

  if (ab == 0) {
    cerr << "Server: Cannot initialise addressbook. Exiting." << endl;
    exit(1);
  }

  while (true) {
    
    int fd = acceptOnSocket(server_fd);
    
    if (fd == -1) {
      cerr << "Server: Accept failed" << endl;
      ::sleep(1);
      continue;
    }
    
#if 0
    ssize_t i = ::write(fd, welcomeMessage, welcomeMessage.length());
    
    if (i == -1) {
      perror("send");
      ::close(fd);
      exit(1);
    }
#endif

    if (fork() == 0) {
      // Child process.
      cerr << "Server: Hello I'm a child process serving requests !" << endl;
      while (processCommand(fd));
      ::close(fd);
      cerr << "Server: Goodbye." << endl;
    }

    ::close(fd);
    
    while (::waitpid(-1, NULL, WNOHANG) > 0);
  }
  
  ::close(server_fd);
}

  void
doReplace(int fd, const QByteArray & s)
{
  char * d = s.data();
  
  Q_UINT32 sizeOfEntity = decodeToInt(d);

  // The size of the key is what's left from the total after the entity.
  Q_UINT32 sizeOfKey    = s.size() - sizeOfEntity - 4;

  // Sanity check
  if (4 + sizeOfKey + sizeOfEntity != s.size()) {
    // Ensure that the sizes given correspond to the actual data size.
    cerr << "Server: doReplace: Data size is incorrect" << endl;
  }

  QByteArray key;
  key.setRawData(d + 4, sizeOfKey);
  // We now have the key.
  // Last char of key gives us the type.
  char entityType = key[key.size() - 1];
  
  QByteArray entityData;
  entityData.setRawData(d + 4 + sizeOfKey, sizeOfEntity);
  
  // We now have data to make the entity with.
  // Use the entity type to decide which type of entity to make.
  KAB::Entity * e;
  if (entityType == 'e') e = new KAB::Entity();
  else e = new KAB::Group();
  
  // Create a data stream (using the data for the entity) and tell
  // the entity to load itself from that stream.
  QDataStream str(entityData, IO_ReadOnly);
  e->load(str);
  
  // Remember to do 'resetRawData'
  key.resetRawData(d + 8, sizeOfKey);
  entityData.resetRawData(d + 8 + sizeOfKey, sizeOfEntity);

  // Write the new entity to the addressbook.
  ab->write(e);

  char c = '0';
  ::write(fd, &c, 1);
}

  void
doErase(int fd, const QByteArray & s)
{
  char * d = s.data();
  Q_UINT32 sizeOfKey = decodeToInt(d);
  
  QByteArray key;
  
  key.setRawData(d + 4, sizeOfKey);
  
  bool ok = ab->remove(QString(key));
  
  key.resetRawData(d + 4, sizeOfKey);
  
  char c = ok ? '0' : '1';
  ::write(fd, &c, 1);
}

  void
doAdd(int fd, const QByteArray & s)
{
  cerr << "Server: Adding entity" << endl;
  char * d = s.data();
  
  Q_UINT32 sizeOfEntity = s.size() - 1;
  
  QByteArray entityData;
  entityData.setRawData(d + 1, sizeOfEntity);
  
  // We now have data to make the entity with.
  // Use the entity type to decide which type of entity to make.
  KAB::Entity * e;
  if (d[0] == 'e') e = new KAB::Entity();
  else e = new KAB::Group();
  
  // Create a data stream (using the data for the entity) and tell
  // the entity to load itself from that stream.
  QDataStream str(entityData, IO_ReadOnly);
  cerr << "Server: ,,," << endl;
  e->load(str);
  cerr << "Server: ,,," << endl;
  cerr << "Server: Loaded entity from stream" << endl;
  cerr << "Server: Entity's name is \"" << e->name() << "\"" << endl;
  
  // Remember to do 'resetRawData'
  entityData.resetRawData(d + 1, sizeOfEntity);

  // Write the new entity to the addressbook.
  ab->write(e);

  cerr << "Server: Writing entity to backend" << endl;
  char c = '0';
  ::write(fd, &c, 1);
}

  void
doFind(int fd, const QByteArray & s)
{
  QByteArray key;
  key.setRawData(s.data(), s.size());

  KAB::Entity * e = ab->entity(QString(key));

  key.resetRawData(s.data(), s.size());

  if (e == 0) {
    char c = 0;
    for (int i = 0 ; i < 4; i++)
      ::write(fd, &c, 1);
  }

  QByteArray entityAsByteArray;
  QDataStream stream(entityAsByteArray, IO_WriteOnly);
  e->save(stream);

  Q_UINT32 sz = entityAsByteArray.size();
  
  char * c = fourOctets(sz);
  ::write(fd, c, 4);
  delete [] c;
}

  void
doSearch(int fd, const QByteArray & s)
{
  QByteArray key;
  key.setRawData(s.data(), s.size());
  key.resetRawData(s.data(), s.size());
  for (int i = 0; i < 4; i++) {
    char c = 0;
    ::write(fd, &c, 1);
  }
}

  void
doList(int fd)
{
  cerr << "Server: Doing listing" << endl;
  QStrList l = ab->allKeys();
  cerr << "There are " << l.count() << " keys in this addressbook" << endl;

  int sz = l.count();
  
  char * c = fourOctets(sz);
  fprintf(stderr, "c == %d %d %d %d\n", c[0], c[1], c[2], c[3]);
  ::write(fd, c, 4);
  delete [] c;
  c = 0;
  
  QStrListIterator it(l);
  
  for (; it.current(); ++it) {
    
    QCString s(it.current());
    
    sz = s.length();
    c = fourOctets(sz);
    ::write(fd, c, 4);
    delete [] c;
    c = 0;

    ::write(fd, s, s.length());
  }
  cerr << "Server: Done listing" << endl;
}
 

