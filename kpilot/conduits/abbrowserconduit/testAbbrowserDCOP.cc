#include <unistd.h> // for usleep
#include <assert.h>
#include <qdict.h>

#include <qstringlist.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kdebug.h>
#include <krun.h>


#include "contactentry.h"


void output(const QStringList &strlist)
    {
    for (QStringList::ConstIterator iter = strlist.begin();
	 iter != strlist.end();
	 ++iter)
	qDebug("\t%s",(*iter).latin1());
    }


int main(int argc, char *argv[])
    {
		  
    //FUNCTIONSETUP;
    KApplication app(argc, argv, "testAbbroserDCOP", false, false);

    //app:
    DCOPClient *dcopptr = app.dcopClient();
    if (!dcopptr)
	{
	kdWarning() << "Can't get DCOP client."
		    << endl;
	return 1; 
	}
    //dcopptr->registerAs("testAbbrowserDCOP");
    if (!dcopptr->attach())
	{
	kdWarning() << "Unable to attach to DCOP" << endl;
	return 2;
	}
    
    qDebug("Attached to DCOP server");

    QByteArray sendData;
    QByteArray replyData;
    QCString replyTypeStr;
    if (!dcopptr->call("kaddressbook", "KAddressBookIface", "interfaces()",
		       sendData, replyTypeStr, replyData))
	{
	// abbrowser not running, start it
	KURL::List noargs;
	KRun::run("kaddressbook", noargs);

	qDebug("Waiting to run abbrowser");
	sleep(15); // 5000 seems to work for greg??
	qDebug("Back from wait");
	
	if (!dcopptr->call("kaddressbook", "KAddressBookIface", "interfaces()",
			   sendData, replyTypeStr, replyData))
	    {
	    kdWarning() << "Unable to call abbrowser interfaces()" << endl;
	    return 3;
	    }
	}
    if (!dcopptr->call("kaddressbook", "KAddressBookIface", "getKeys()",
		       sendData, replyTypeStr, replyData))
	{
	kdWarning() << "Unable to call abbrowser getKeys()" << endl;
	return 4;
	}
    assert(replyTypeStr == "QStringList");

    QStringList keys;
    QDataStream keyStream(replyData, IO_ReadOnly);
    keyStream >> keys;
    qDebug("kaddressbook keys");
    output(keys);

    if (argc > 1)
	{
	ContactEntry testAddEntry;
	testAddEntry.setNamePrefix("Dr.");
	testAddEntry.setFirstName("John");
	testAddEntry.setLastName("Doe");
	testAddEntry.setName();
	testAddEntry.setEmail("dow@nowhere.com");
	testAddEntry.setJobTitle("Computer Scientist");
	testAddEntry.setCustomField("KPilot_id", QString::number(5000));
	testAddEntry.setFolder("Friends");
	ContactEntry::Address *a = testAddEntry.getHomeAddress();
	a->setStreet("100 No Where Ave.");
	
	QByteArray sendEntryData;
	QDataStream sendEntryStream(sendEntryData, IO_WriteOnly);
	sendEntryStream << testAddEntry;
	
	if (!dcopptr->call("kaddressbook", "KAddressBookIface", "addEntry(ContactEntry)",
			   sendEntryData, replyTypeStr, replyData))
	    {
	    kdWarning() << "Unable to call abbrowser addEntry" << endl;
	    return 9;
	    }
	}
    
    if (!dcopptr->call("kaddressbook", "KAddressBookIface", "getEntryDict()",
		       sendData, replyTypeStr, replyData))
	{
	kdWarning() << "Unable to call abbrowser getEntryDict()" << endl;
	return 5;
	}
    assert(replyTypeStr == "QDict<ContactEntry>");
    
    QDataStream dictStream(replyData, IO_ReadOnly);
    QDict<ContactEntry> entries;
    dictStream >> entries;

    // deleting all doe's
    qDebug("QDict<ContactEntry> count = %d", entries.count());
    for (QDictIterator<ContactEntry> iter(entries);iter.current();++iter)
	{
	qDebug("entry %d = ", iter.currentKey().latin1());
	//iter.current()->debug();
	
	ContactEntry *e = iter.current();
	//qDebug("first name = %s", e->getFirstName().latin1());
	//qDebug("last name = %s", e->getLastName().latin1());
	//qDebug("middle name = %s", e->getMiddleName().latin1());
	//qDebug("email = %s", e->getEmail().latin1());
	if (e->getLastName() == "Doe")
	    {
	    if (argc == 1)
		{
		qDebug("Removing entry");
		e->debug();
		QByteArray sendRemoveKey;
		QDataStream removeStream(sendRemoveKey, IO_WriteOnly);
		removeStream << iter.currentKey();
		if (!dcopptr->call("kaddressbook", "KAddressBookIface",
				   "removeEntry(QString)",
				   sendRemoveKey, replyTypeStr, replyData))
		    {
		    kdWarning() << "Unable to call abbrowser removeEntry" << endl;
		    return 9;
		    }
		}
	    else
		{
		e->setFirstName("Sally");
		e->setName();
		QByteArray sendChangeName;
		QDataStream changeStream(sendChangeName, IO_WriteOnly);
		changeStream << iter.currentKey();
		changeStream << *e;
		if (!dcopptr->call("kaddressbook", "KAddressBookIface",
				   "changeEntry(QString, ContactEntry)",
				   sendChangeName, replyTypeStr, replyData))
		    {
		    kdWarning() << "Unable to call abbrowser changeEntry" << endl;
		    return 10;
		    }
		}
	    }
	}
    
    // save the changes
    if (!dcopptr->call("kaddressbook", "KAddressBookIface", "save()",
		       sendData, replyTypeStr, replyData))
	{
	kdWarning() << "Unable to call abbrowser save()" << endl;
	return 5;
	}
    
    dcopptr->detach();
    return 0;
    }
