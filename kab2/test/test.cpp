#include <iostream>
#include <qstring.h>
#include <qcstring.h>

#include <kapp.h>

#include <Kab2.h>
#include <KabEntity.h>

  int
main(int argc, char ** argv)
{
  new KApplication(argc, argv, "test");

  QString urlStr(argv[1]);

  KURL url(urlStr);

  KAB::Kab2 * ab = KAB::Kab2::create();

  if (ab == 0) {
    cerr << "Kab2 object is 0!" << endl;
    exit(1);
  }

  cerr << "Initialised kab2 library" << endl;

  QList<KAB::AddressBookClient> addressBookList = ab->addressBookList();

  cerr << addressBookList.count() << " addressbook(s) available" << endl;

  if (addressBookList.count() == 0) {
    exit(0);
  }

  QListIterator<KAB::AddressBookClient> it(addressBookList);

  for (; it.current(); ++it) {
    cerr << "AddressBook: " << it.current()->name() << endl;
  
    cerr << "  Key list:" << endl;

    QStrList l = it.current()->allKeys();

    QStrListIterator sit(l);

    for (; sit.current(); ++sit) {
      cerr << "  Key:  " << sit.current() << endl;
    }
  }

  cerr << "Now creating a new entity" << endl;

  KAB::AddressBookClient * client = addressBookList.at(0);

  KAB::Entity * e = new KAB::Entity;

  char d[10] = "Test data";
  
  QString name = "This is a field name";
  QByteArray data;
  data.setRawData(d, 10);
  QString mimeType = "text";
  QString mimeSubType = "plain";

  cerr << "Adding new field to entity with name \"" << name << "\"" << endl;
  e->add(name, data, mimeType, mimeSubType);
  
  data.resetRawData(d, 10);
  
  cerr << "Asking client to write entity" << endl;
  bool retval = client->write(e);
  cerr << "Client returned " << (retval ? "OK" : "ERROR") << endl;
  
  cerr << "Key list for first addressbook is now:" << endl;
  
  QStrList l = client->allKeys();

  cerr << "I have a string list with " << l.count() << " entries" << endl;

  QStrListIterator sit(l);

  for (; sit.current(); ++sit) {
    cerr << "  Key:  " << sit.current() << endl;
  }
}

