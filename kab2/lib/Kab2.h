#include <KabAddressBookClient.h>

#ifndef KAB2_H
#define KAB2_H

namespace KAB
{
  
class Kab2
{
  public:

    static Kab2 * create();

    Kab2();
    ~Kab2();

    QList<AddressBookClient> addressBookList()
    {
      return addressBookList_;
    }
    
    void addAddressBook(
      const QString & name, const QString & format, const QString & location);
    void removeAddressBook(const QString & name);
    
  private:

    QList<AddressBookClient> addressBookList_;
    static Kab2 * KAB2;

};
}
#endif

