#include "agendacontacts.h"

#include <kstandarddirs.h>


AgendaContacts::AgendaContacts()
{
	filename = locateLocal("data","kitchensync/agenda/contacts.dat");
}

AgendaContacts::~AgendaContacts()
{
}
