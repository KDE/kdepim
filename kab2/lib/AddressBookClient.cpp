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
decodeToInt(unsigned char * s)
{
  Q_UINT32 i = 0uL;
  i |= s[0] << 24;
  i |= s[1] << 16;
  i |= s[2] << 8;
  i |= s[3];
  return i;
}

  unsigned char *
fourOctets(Q_UINT32 i)
{
  unsigned char * s = new unsigned char[4];

  s[0] = (i & 0xff000000) >> 24;
  s[1] = (i & 0x00ff0000) >> 16;
  s[2] = (i & 0x0000ff00) >> 8;
  s[3] =  i & 0x000000ff;

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
  quit();
  _disconnect();
}

  void
AddressBookClient::quit()
{
  char q = 'q';
  int sz = ::write(fd, &q, 1);
  if (sz != 1)
    perror("write");
}

  Entity *
AddressBookClient::entity(const QString & key)
{
  cerr << "Entity with key \"" << key << "\" requested" << endl;
  const char * a = key.ascii();
  int l = strlen(a);
  int total_out = 5 + l;
  
  char * s = new char[total_out];
  
  // We need to write 'f' [keylength] [key]
  s[0] = 'f';

  unsigned char * c = fourOctets(l);
  memcpy(s + 1, c, 4);
  delete [] c;
  c = 0;
  
  memcpy(s + 5, a, l);
  
  cerr << "Writing request for entity \"" << key << "\"" << endl;
  int sz = ::write(fd, s, total_out);

  if (sz != total_out) {
    cerr << "AddressBookClient::entity(): Couldn't write the correct number of bytes" << endl;
    // Argh
  }
    
  cerr << "Reading response" << endl;
  // We now read back [total] [entity]
  c = new unsigned char[4];
  sz = ::read(fd, c, 4);

  int entitySize = decodeToInt(c);
  delete [] c;
  c = 0;
  
  cerr << "entitySize is " << entitySize << endl;

  if (sz != 4) {
    cerr << "AddressBookClient::entity(): Couldn't read the correct number of bytes" << endl;
    return 0;
  }

  if (entitySize == 0) {
    cerr << "AddressBookClient::entity(): entitySize == 0 -> not found" << endl;
    // Not found.
    return 0;
  }

  char * buf = new char [entitySize];

  sz = ::read(fd, buf, entitySize);

  if (sz != entitySize) {
    cerr << "AddressBookClient::entity(): entitySize != what I was expecting" << endl;
    return 0;
  }

  cerr << "Creating new entity with data" << endl;
  QByteArray qbuf;
  qbuf.setRawData(buf, entitySize);
  QDataStream stream(qbuf, IO_ReadOnly);
  Entity * e = 0;
  
  if (key.right(1) == "e")
    e = new Entity;
  else
    e = new Group;
  
  e->load(stream);
  
  qbuf.resetRawData(buf, entitySize);

  delete [] buf;
  buf = 0;
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

  QDataStream s2(a, IO_ReadOnly);
  e->load(s2);
  cerr << "Entity read back has name \"" << e->name() << "\"" << endl;

  cerr << "Entity id == \"" << e->id() << "\"" << endl;
  
  const char * entity = a.data();
  int entitySize = a.size();
  
  const char * key = e->id().ascii();
  int keySize = strlen(key);
  cerr << "Entity size     == " << entitySize << endl;
  cerr << "Key size        == " << keySize << endl;
  
  // We need to send 'a' [total] [type] [keySize] [key] [entity]

  // Create the 'total' field - the server will be expecting a number which
  // specifies exactly how many octets it needs to read.
  Q_UINT32 t = 1 + 4 + keySize + entitySize;
  
  cerr << "Total info size == " << t << endl;
  
  unsigned char * total_size = fourOctets(t);
  unsigned char * key_size = fourOctets(keySize);

  fprintf(stderr, "total_size: %d %d %d %d\n", total_size[0], total_size[1], total_size[2], total_size[3]);

  cerr << "decoded total_size == " << decodeToInt(total_size) << endl;
  cerr << "decoded key_size   == " << decodeToInt(key_size) << endl;

  // The total amount of octets we're going to write. The server will first
  // read the command name (here 'a') and the total (4 bytes -> uint32).
  int total_out = 1 + 4 + t; // 'a' [total] t
  
  cerr << "Total data size == " << total_out << endl;
  
  int offset = 0;
  
  char * out = new char [total_out];
  // Command name.
  *(out + offset) = 'a';
 
  offset += 1;
 
  // Total size.
  memcpy(out + offset, total_size, 4);
  
  offset += 4;

  // Entity type.
  char type = (e->type() == EntityTypeEntity) ? 'e' : 'g';
  cerr << "AddressBookClient::write(): entity type is " << type << endl;
  *(out + offset) = type;

  offset += 1;

  // Key size.
  memcpy(out + offset, key_size, 4);

  offset += 4;
  
  memcpy(out + offset, key, keySize);
  
  offset += keySize;
  
  memcpy(out + offset, entity, entitySize);

  cerr << "Writing " << entitySize << " bytes" << endl;

  int sz = ::write(fd, out, total_out);

  if (sz != total_out) {
    cerr << "Wrote " << sz << " bytes" << endl;
    cerr << "AddressBookClient::write(): Couldn't write the correct number of bytes" << endl;
    delete [] total_size;
    total_size = 0;
    delete [] key_size;
    key_size = 0;
    delete [] out;
    out = 0;
    return false;
  }

  cerr << "Wrote " << sz << " bytes" << endl;

  char retval;

  cerr << "Reading retval" << endl;
  sz = ::read(fd, (void *)&retval, (size_t)1);
  if (sz == -1)
    perror("read");
  cerr << "Read " << sz << " bytes" << endl;
  cerr << "Retval == '" << retval << "'" << endl;

  if (sz != 1) {
    cerr << "AddressBookClient::write(): Couldn't read the correct number of bytes" << endl;
    delete [] total_size;
    total_size = 0;
    delete [] key_size;
    key_size = 0;
    delete [] out;
    out = 0;
    return false;
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

  cerr << "AddressBookClient: remove(): key length is " << l << endl;
  
  char * s = new char[5 + l];
  s[0] = 'e';
  
  unsigned char * c = fourOctets(l);
  
  memcpy(s + 1, c, 4);
 
  memcpy(s + 5, a, l);
  
  int sz = ::write(fd, s, 5 + l);
  
  if (sz != 5 + l) {
    cerr << "AddressBookClient::remove(): Couldn't write the correct number of bytes" << endl;
    delete [] s;
    s = 0;
    return false;
  }

  char retval;
  sz = ::read(fd, &retval, 1);

  if (sz != 1) {
    cerr << "AddressBookClient::remove(): Couldn't read the correct number of bytes" << endl;
    delete [] s;
    s = 0;
    return false;
  }

  cerr << "AddressBookClient::remove(): retval == '" << retval << "'" << endl;

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
    cerr << "AddressBookClient::allKeys(): Couldn't write the correct number of bytes" << endl;
    return l;
  }

  unsigned char c[4];
  sz = ::read(fd, c, 4);
  
  if (sz != 4) {
    cerr << "AddressBookClient::allKeys(): Couldn't read the correct number of bytes" << endl;
    return l;
  }

  Q_UINT32 length;
  length = decodeToInt(c);
  cerr << "length == " << length << endl;

  if (length == 0) {
    cerr << "AddressBookClient::allKeys(): No keys in addressbook" << endl;
    return l;
  }

  for (Q_UINT32 i = 0; i != length; i++) {

    sz = ::read(fd, c, 4);

    int keyLength = decodeToInt(c);

    if (keyLength == 0) {
      cerr << "AddressBookClient::allKeys(): Key length is 0 ?!?" << endl;
      return l;
    }
    
    char * buf = new char [keyLength + 1];

    sz = ::read(fd, buf, keyLength);

    if (sz != keyLength) {
      cerr << "AddressBookClient::allKeys(): Key length doesn't match read bytes" << endl;
      return l;
    }

    // NUL-terminate key.
    buf[keyLength] = '\0';
    
    l.append(buf);

    delete [] buf;
    buf = 0;
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
  if (-1 == ::close(fd))
    perror("close");
}

