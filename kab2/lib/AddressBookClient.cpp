#include <iostream>

#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>

#include "KabAddressBookClient.h"
#include "KabGroup.h"
#include "KabEntity.h"

  inline Q_UINT32
decodeToInt(char * s)
{ return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3]; }


using namespace KAB;

AddressBookClient::AddressBookClient(const QString & name, const KURL & url)
      : name_   (name),
        url_    (url)
{
  cerr << "Initialising client" << endl;
  cerr << "  My name is " << name << endl;
  // openConnection();
}
        
AddressBookClient::~AddressBookClient()
{
  // closeConnection();
}

  Entity *
AddressBookClient::entity(const QString & key)
{
}

  Group *
AddressBookClient::group(const QString & key)
{
  return (Group *)entity(key);
}
 
  void
AddressBookClient::write(Entity * e)
{
  QByteArray a;
  QDataStream s(a, IO_WriteOnly);
  e->save(s);
}

  bool
AddressBookClient::remove(const QString & key)
{
  char * a = key.ascii();
  int l = strlen(a);
  char * s = new char(5 + l);
  s[0] = 'e';
  char c[4];
  int size = l;
  c[0] = size >> 24;
  c[1] = size >> 16;
  c[2] = size >> 8;
  c[3] = size;
  memcpy(s + 5, a, l);
//  sendData(s);
}
  
  void
AddressBookClient::update(Entity * e)
{
  QByteArray a;
  QDataStream s(a, IO_WriteOnly);
  e->save(s);
}

  QStrList
AddressBookClient::allKeys()
{
  QByteArray a;
  a.assign("l", 1);
//  sendData("l");
}


