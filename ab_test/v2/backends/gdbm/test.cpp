#include <iostream>
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include "kab_gdbm_backend.h"

// Testing using gdbm as a kab backend.
// Rik Hemsley <rik@kde.org>
// 
// Quite a simple API here. An object knows how to stream itself and also
// how to make a QByteArray from its own streamed representation and write
// that to a subclass of KabBackend.
// 
// This means all we need to do is to say to an Entity 'save yourself
// using this key to this KabBackend' and it returns true if it worked.
//
// The relevant methods are:
// bool Entity::read(const QCString & key, KabBackend * backend);
// and
// bool Entity::write(const QCString & key, KabBackend * backend);
// 
// Added an example search facility while looks for an Entity with the
// given name.

main()
{
  {
  KabGDBMBackend db;
  
  bool retval = false;
  
  QFile f(".a");
  f.open(IO_ReadOnly);
  
  QTextStream t(&f);
  
  while (!t.eof()) {

    QString s = t.readLine();
    Entity e;
    e.setName(s.left(s.find(' ')));
    e.setData(s);
    QCString key = e.name().ascii();
    retval = e.write(key, &db);
  
    if (!retval)
      cerr << "Unable to write" << endl;
  }

  QTime start(QTime::currentTime());
  Entity * e = db.search("tty0");
  cerr << "Search took " << start.msecsTo(QTime::currentTime()) << endl;
  
  if (e == 0)
    cerr << "Not found" << endl;
  else
    cerr << "Found: " << e->data() << endl;
  
  delete e;
}
}

