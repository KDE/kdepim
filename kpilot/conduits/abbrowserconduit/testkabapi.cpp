#include <kabapi.h>
#include <kapp.h>
#include <addressbook.h>

using namespace std;

int main(int argc, char *argv[])
    {
    KApplication app(argc, argv, "testkabapi", false, true);
    
    KabAPI *kab = new KabAPI(NULL,"kab");
    kab->init(); // open the db
    //if (kab->exec())
    //qDebug("selected");
    
    AddressBook *book = kab->addressbook();
    if (!book)
	{
	qWarning("Unable to get AddressBook");
	return 1;
	}
    
    list<AddressBook::Entry> entryList;
    int error = book->getEntries(entryList);
    if (error != AddressBook::NoError)
	{
	qWarning("Unable to get entry list; error %d", error);
	switch (error)
	    {
	    case AddressBook::PermDenied :
		qWarning("PermDenied");
		break;
	    case AddressBook::Locked :
		qWarning("Locked");
		break;
	    case AddressBook::Rejected :
		qWarning("Rejected");
		break;
	    case AddressBook::NoSuchEntry :
		qWarning("NoSuchEntry");
		break;
	    case AddressBook::NoEntry :
		qWarning("NoEntry");
		break;
	    case AddressBook::NoFile :
		qWarning("NoFile");
		break;
	    case AddressBook::InternError :
		qWarning("InternError");
		break;
	    case AddressBook::OutOfRange :
		qWarning("OutOfRange");
		break;
	    case AddressBook::NotImplemented :
		qWarning("NotImplemented");
		break;
	    }
		
	return 1;
	}
	
    qDebug("Got entry list with size %d", entryList.size());
    qDebug("Address book size %d", book->noOfEntries());
    
    for (list<AddressBook::Entry>::iterator iter = entryList.begin();
	 iter != entryList.end();
	 ++iter)
	{
	qDebug("\t%s", (*iter).lastname.latin1());
	}
    
    return 0;
    }
