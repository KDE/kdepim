#include "contact.h"
#include "contactentry.h"
#include "datepickerdialog.h"
#include "namevalue.h"
#include "AddressBookWidget.h"
#include <kapp.h>
#include <qwidget.h>

main(int argc, char * argv[])
{
	KApplication * app = new KApplication(argc, argv, "Addressbook test");
	
    ContactEntry * ce;
	ce = new ContactEntry("entry.txt");
    
    AddressBookDialog * w = new AddressBookDialog(ce, "Addressbook test");
    
    app->setMainWidget(w);
	app->exec();
}

