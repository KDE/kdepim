#ifndef AgendaBase_H
#define AgendaBase_H

#include <qstring.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <db1/db.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


class AgendaBase : public QObject
{
	Q_OBJECT

public:
	AgendaBase();
	~AgendaBase();

	void FileToAgenda(); 
	void AgendaToFile(); 
	void FileToList();
	void ListToFile();
	

private:
	QString filename;
	QPtrList<QStringList> dblist;
	DB *database;
	DBT key, value;




};

#endif
