#include <iostream>
#include <qstring.h>
#include <qstringlist.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kurl.h>

#include <Kab2.h>
#include <KabAddressBookClient.h>

using namespace KAB;

Kab2 * Kab2::KAB2 = 0;

  Kab2 *
Kab2::create()
{
  if (Kab2::KAB2 == 0)
    Kab2::KAB2 = new Kab2;

  return Kab2::KAB2;
}

Kab2::Kab2()
{
  addressBookList_.setAutoDelete(true);
  
  cerr << "Locate sez " << locate("config", "kab2rc") << endl;
  KConfig c(locate("config", "kab2rc"));
  c.setGroup("General");
  QStringList addressBookNames = c.readListEntry("AddressBookNames");
  cerr << "Kab2::Kab2: There are " << addressBookNames.count()
    << " addressbooks to load" << endl;

  QStringList::ConstIterator it;

  for (it = addressBookNames.begin(); it != addressBookNames.end(); ++it) {

    cerr << "Loading addressbook \"" << *it << "\"" << endl;
    c.setGroup(*it);
    QString name = c.readEntry("Name");
    QString location = c.readEntry("Location");
    cerr << "Location \"" << location << "\"" << endl;

    AddressBookClient * client =
      new AddressBookClient(name, location);

    addressBookList_.append(client);
  }
}

Kab2::~Kab2()
{
  KAB2 = 0;
}

