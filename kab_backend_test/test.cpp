#include <iostream>
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

main()
{
  KabGDBMBackend db;
  
  bool retval = false;

  Entity e;
  e.setName("This is a test entity");
  e.setData("Here is some test data");
  
  QCString key = "This is my key !";
  
  retval = e.write(key, &db);
  
  if (!retval)
    cerr << "Unable to write" << endl;
  
  Entity e2;
  
  retval = e2.read(key, &db);
  
  if (!retval)
    cerr << "Unable to read" << endl;
  
  cerr << "name: " << e2.name() << endl;
  cerr << "data: " << e2.data() << endl;
}

