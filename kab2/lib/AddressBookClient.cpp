#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>

#include <iostream>

#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>

#include "KabAddressBookClient.h"
#include "KabGroup.h"
#include "KabEntity.h"

const int PORT = 6566; // AB

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

using namespace KAB;

AddressBookClient::AddressBookClient(const QString & name, const KURL & url)
      : name_   (name),
        url_    (url)
{
  cerr << "Initialising client" << endl;
  cerr << "  My name is " << name << endl;
  _connect();
}
        
AddressBookClient::~AddressBookClient()
{
  _disconnect();
}

  Entity *
AddressBookClient::entity(const QString & key)
{
  const char * a = key.ascii();
  int l = strlen(a);
  char * s = new char(5 + l);
  s[0] = 'e';
  char * c = fourOctets(l);
  memcpy(s + 1, c, 4);
  memcpy(s + 5, a, l);
  int total_out = 1 + 4 + l;
  int sz = ::write(fd, a, total_out);

  if (sz != total_out) {
    // Argh
  }
    
  delete [] c;
  c = 0;

  c = new char[4];
  sz = ::read(fd, c, 4);

  if (sz != 4) {
    // Argh
  }

  int entitySize = decodeToInt(c);
  
  delete [] c;
  c = 0;

  if (entitySize == 0) {
    // Not found.
    return 0;
  }

  char * buf = new char [entitySize];

  sz = ::read(fd, buf, entitySize);

  if (sz != entitySize) {
    // Argh
  }

  QByteArray qbuf;
  qbuf.setRawData(buf, entitySize);
  QDataStream stream(qbuf, IO_ReadOnly);
  Entity * e = new Entity;
  e->load(stream);
  qbuf.resetRawData(buf, entitySize);

  return e;
}

  Group *
AddressBookClient::group(const QString & key)
{
  Entity * e = entity(key);
  return (e == 0 ? 0 : (Group *)e);
}
  
  bool
AddressBookClient::write(Entity * e)
{
  cerr << "Writing an entity with name \"" << e->name() << "\"" << endl;
  QByteArray a;
  QDataStream s(a, IO_WriteOnly);
  e->save(s);
  
  const char * entity = a.data();
  int entitySize = a.size();
  
  const char * key = e->id().ascii();
  int keySize = strlen(key);
  
  // We need to send 'a' [total] [type] [keySize] [key] [entity]

  // type keySize keySize entitySize
  Q_UINT32 t = 1 + 4 + keySize + entitySize;
  
  char * total_size = fourOctets(t);
  char * key_size = fourOctets(keySize);

  int total_out = 1 + 4 + t; // 'a' [total] t
  char * out = new char [total_out];
  out[0] = 'a';
  memcpy(out + 1, total_size, 4);
  char type = (e->type() == (EntityTypeEntity ? 'e' : 'g'));
  out[5] = type;
  memcpy(out + 6, key_size, 4);
  memcpy(out + 10, key, keySize);
  memcpy(out + 10 + keySize, entity, entitySize);

  int sz = ::write(fd, out, total_out);

  if (sz != total_out) {
    // Argh
  }

  char retval;

  sz = ::read(fd, &retval, 1);

  if (sz != 1) {
    // Argh
  }

  delete [] total_size;
  total_size = 0;
  delete [] key_size;
  key_size = 0;
  delete [] out;
  out = 0;

  return (retval == '0');
}

  bool
AddressBookClient::remove(const QString & key)
{
  const char * a = key.ascii();
  int l = strlen(a);
  
  char * s = new char(5 + l);
  s[0] = 'e';
  
  char * c = fourOctets(l);
 
  memcpy(s + 5, a, l);
  
  int sz = ::write(fd, s, l);
  
  if (sz != l) {
    // Argh
  }

  char retval;
  sz = ::read(fd, &retval, 1);

  if (sz != 1) {
    // Argh
  }

  delete [] c;
  c = 0;

  return (retval == '0');
}
  
  bool
AddressBookClient::update(Entity * e)
{
  QByteArray a;
  QDataStream s(a, IO_WriteOnly);
  e->save(s);
  return false;
}

  QStrList
AddressBookClient::allKeys()
{
  cerr << "Doing allKeys()" << endl;
  QStrList l;
  l.setAutoDelete(true);

  char el = 'l';
  ssize_t sz = ::write(fd, (void *)&el, 1);

  if (sz != 1) {
    // Argh
    return l;
  }

  char c[4];
  sz = ::read(fd, c, 4);
  
  if (sz != 4) {
    // Argh 
    return l;
  }

  Q_UINT32 length;
  length = decodeToInt(c);
  cerr << "length == " << length << endl;

  if (length == 0)
    return l;

  for (Q_UINT32 i = 0; i != length; i++) {

    sz = ::read(fd, c, 4);

    int keyLength = decodeToInt(c);

    if (keyLength == 0) {
      // Argh
      return l;
    }
    
    char * buf = new char [keyLength + 1];

    sz = ::read(fd, buf, keyLength);

    if (sz != keyLength) {
      // Argh
      return l;
    }

    // NUL-terminate key.
    buf[keyLength] = '\0';
    
    l.append(buf);

    delete [] buf;
  }

  return l;
}

  bool
AddressBookClient::_connect()
{  
  struct hostent * he;
  struct sockaddr_in serverAddress;

  cerr << "url.host() == " << url_.host() << endl;

  he = ::gethostbyname(url_.host());
  
  if (!he) {
    cerr << "Couldn't gethostbyname" << endl;
    perror("gethostbyname");
    return false;
  }

  fd = ::socket(AF_INET, SOCK_STREAM, 0);

  if (fd == -1) {
    cerr << "Couldn't create a client socket" << endl;
    perror("socket");
    return false;
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT);
  serverAddress.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(serverAddress.sin_zero), 0, 8);

  int connected =
    ::connect(
      fd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));

  if (connected == -1) {
    cerr << "Couldn't connect" << endl;
    perror("socket");
    return false;
  }

  return true; 
}

  void
AddressBookClient::_disconnect()
{
  ::close(fd);
}

