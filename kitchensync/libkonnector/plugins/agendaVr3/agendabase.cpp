#include "agendabase.h"

#include <db1/db.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qptrlistiterator.h>


AgendaBase::AgendaBase()
{
}

AgendaBase::~AgendaBase()
{
}

void AgendaBase::FileToAgenda()
{
}

void AgendaBase::AgendaToFile()
{
}

void AgendaBase::FileToList()
{
	database = dbopen(filename, O_RDONLY, 0644, DB_HASH, NULL);
	if (!database)
		return;
	for (int i = database->seq(database, &key, &value, R_FIRST); i == 0; i = database->seq(database, &key, &value, R_NEXT))
	{
		dblist.append( QStringList::split( 0x1F, (char *)value.data ) );
	}
	database->close(database);
}

void AgendaBase::ListToFile()
{
	database = dbopen(filename, O_RDWR, 0644, DB_HASH, NULL);
	int i = 0;
	for (QPtrListIterator<QStringList> itPtr( dblist ); itPtr.current(); ++itPtr)
	{
		++i;
		QStrint tmpStr;
		for ( QStringList::Iterator itStr = itPtr.current()->begin(); itStr != itPtr.current->end(); ++itStr )
		{
			tmpStr.append(itStr);		
			tmpStr.append(0x1F);
		}
		key.data = i;
		value.data = tmpStr;
		database->put(database, &key, &value, 0);
	}
	database->close(database);
}


#include "agendabase.moc"
