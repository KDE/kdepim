
#include <dlfcn.h>

#include <iostream>

#include <qfile.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>

#include "KabAddressBook.h"
#include "KabPerson.h"
#include "KabLocation.h"
#include "KabGroup.h"
#include "KabEntity.h"

  bool
KAB::AddressBook::_initBackend()
{
  // Try to dynamically load a backend for the specified format.
  cerr << "Loading backend for format '" << format_ << "' ..." << endl;
    
  QCString backendFilename =
    QCString("libkab" + QCString(format_.lower().ascii()) + "backend.so");
    
  KabBackendHandle handle(::dlopen(backendFilename, RTLD_GLOBAL | RTLD_LAZY));
    
  if (!handle) {
    cerr << "Couldn't open backend plugin " << backendFilename << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    usable_ = false;
    return false;
  }

  backendInit _backendInit =
    (backendInit)::dlsym(handle, "KabBackendInit");
  
  backendRead _backendRead =
    (backendRead)::dlsym(handle, "KabBackendRead");
 
  backendWrite _backendWrite =
    (backendWrite)::dlsym(handle, "KabBackendWrite");
     
  backendRemove _backendRemove =
    (backendRemove)::dlsym(handle, "KabBackendRemove");
  
  backendAllKeys _backendAllKeys =
    (backendAllKeys)::dlsym(handle, "KabBackendAllKeys");

  cerr << "Successfully loaded backend." << endl;
  cerr << "Resolving symbols ..." << endl;
  
  if (
      !_backendInit   ||
      !_backendRead   ||
      !_backendWrite  ||
      !_backendRemove ||
      !_backendAllKeys) {
        
    cerr << "Unable to init backend for " << format_ << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    ::dlclose(handle);
    usable_ = false;
    return false;
  }
  
  cerr << "Successfully resolved symbols." << endl;
  
  
  init_     = _backendInit;
  read_     = _backendRead;
  write_    = _backendWrite;
  remove_   = _backendRemove;
  allKeys_  = _backendAllKeys;
  handle_   = handle;
  
  cerr << "Initialising ..." << endl;
  init_(url_);
  cerr << "Initialised." << endl;
  usable_ = true;
  return true;
}

  Q_UINT32
KAB::AddressBook::import(const QString & format, const QString & filename)
{
  cerr << "Loading filter plugin for " << format << " using file " << filename << endl; 
  // Try to dynamically load a filter plugin for the specified type.
    
  QCString filterFilename =
    QCString("lib" + QCString(format.lower().ascii()) + "filter.so");
    
  FilterHandle handle(::dlopen(filterFilename, RTLD_LAZY));
    
  if (!handle) {
    cerr << "Couldn't open filter plugin " << filterFilename << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    return 0;
  }
    
  Filter filter((Filter)::dlsym(handle, "import"));
    
  if (!filter) {
    cerr << "Unable to init filter plugin for " << format << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    ::dlclose(handle);
    return 0;
  }
    
  // Now load the data we'll be importing.

  QFile f(filename);
  
  if (!f.open(IO_ReadOnly)) {
    cerr << "Couldn't open import file " << filename << endl;
    return 0;
  }
  
  QCString str;
  
  QTextStream t(&f);

  while (!t.eof())
    str += t.readLine() + '\n';
  
  // Filter !
  Q_UINT32 imported = filter(str, this);
  
  ::dlclose(handle);
  
  return imported;
}

KAB::AddressBook::AddressBook(const QString & format, const KURL & url)
      : format_ (format),
        url_    (url),
        usable_ (false)
{
  _initBackend();
}
        
KAB::AddressBook::AddressBook(const AddressBook & ab)
  : format_ (ab.format_),
    url_    (ab.url_),
    usable_ (false)
{
  _initBackend();
}

  KAB::AddressBook &
KAB::AddressBook::operator = (const KAB::AddressBook & ab)
{
  if (this == &ab) return *this;
  format_ = ab.format_;
  url_    = ab.url_;
  usable_ = false;
  _initBackend();
  return *this;
}

KAB::AddressBook::~AddressBook()
{
  cerr << "Closing backend library" << endl;
  ::dlclose(handle_);
}

  KAB::Entity *
KAB::AddressBook::entity(const QString & id)
{
  EntityType t = keyToEntityType(id.ascii());
  Entity * e = createEntityOfType(t);
  bool readok = e->read(id.ascii(), &read_);
  if (!readok) { delete e; e = 0; }
  return e;
}

  QStrList
KAB::AddressBook::keysOfType(EntityType t)
{
  QStrList l;
  QStrList idList;
  
  allKeys_(idList);
  
  QStrListIterator it(idList);

  for (; it.current(); ++it)
    if (keyToEntityType(it.current()) == t)
      l.append(it.current());
      
  return l;
}

  void
KAB::AddressBook::add(const Entity * e)
{
  e->write(e->id().ascii(), &write_);
}

  bool
KAB::AddressBook::remove(const QString & key)
{
  return remove_(key.ascii());
}
  
  void
KAB::AddressBook::update(Entity * e)
{
  e->write(e->id().ascii(), &write_);
}

  QStrList
KAB::AddressBook::allKeys()
{
  QStrList l;
  allKeys_(l);
  return l;
}

  KAB::EntityType
KAB::AddressBook::keyToEntityType(const QCString & s)
{
  char c = s.at(s.length() - 1);
  EntityType t(EntityTypeEntity);
  
  switch (c) {
    case 'g':           t = EntityTypeGroup;    break;
    case 'p':           t = EntityTypePerson;   break;
    case 'l':           t = EntityTypeLocation; break;
    case 'e': default:  t = EntityTypeEntity;   break;
  }
  return t;
}

  KAB::Entity *
KAB::AddressBook::createEntityOfType(EntityType t)
{
  Entity * e (0);
  switch (t) {
    case EntityTypePerson:          e = new Person;   break;
    case EntityTypeLocation:        e = new Location; break;
    case EntityTypeGroup:           e = new Group;    break;
    case EntityTypeEntity: default: e = new Entity;  break;
  }
  return e;
}

