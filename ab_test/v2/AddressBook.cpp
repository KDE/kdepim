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
#include "KabMember.h"
#include "KabEntity.h"

  bool
KAB::AddressBook::_initBackend()
{
  // Try to dynamically load a backend for the specified format.
  cerr << "Loading backend for format '" << format_ << "' ... ";
    
  QCString backendFilename =
    QCString("libkab" + QCString(format_.lower().ascii()) + "backend.so");
    
  KabBackendHandle handle(::dlopen(backendFilename, RTLD_GLOBAL | RTLD_LAZY));
    
  if (!handle) {
    cerr << " error" << endl
      << "Couldn't open backend plugin " << backendFilename << endl
      << "dlerror() reports: " << ::dlerror() << endl;
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

  cerr << "done" << endl;
  cerr << "Resolving symbols ... ";
  
  if (
      !_backendInit   ||
      !_backendRead   ||
      !_backendWrite  ||
      !_backendRemove ||
      !_backendAllKeys) {
        
    cerr << "error" << endl <<
      "Unable to init backend for " << format_ << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    ::dlclose(handle);
    usable_ = false;
    return false;
  }
  
  cerr << "done" << endl;
  
  
  init_     = _backendInit;
  read_     = _backendRead;
  write_    = _backendWrite;
  remove_   = _backendRemove;
  allKeys_  = _backendAllKeys;
  handle_   = handle;
  
  cerr << "Initialising ... ";
  init_(url_);
  cerr << "done" << endl;
  usable_ = true;
  return true;
}

  Q_UINT32
KAB::AddressBook::import(const QString & format, const QString & filename)
{
  cerr << "Loading filter plugin for " << format << " using file " <<
    filename << " ... "; 
  // Try to dynamically load a filter plugin for the specified type.
    
  QCString filterFilename =
    QCString("lib" + QCString(format.lower().ascii()) + "filter.so");
    
  FilterHandle handle(::dlopen(filterFilename, RTLD_LAZY));
    
  if (!handle) {
    cerr << "error" << endl;
    cerr << "Couldn't open filter plugin " << filterFilename << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    return 0;
  }
    
  Filter filter((Filter)::dlsym(handle, "import"));
    
  if (!filter) {
    cerr << "error" << endl;
    cerr << "Unable to init filter plugin for " << format << endl;
    cerr << "dlerror() reports: " << ::dlerror() << endl;
    ::dlclose(handle);
    return 0;
  }
    
  cerr << "done" << endl;

  // Now load the data we'll be importing.

  QFile f(filename);
  
  cerr << "Importing from file " << filename << " ... ";
  
  if (!f.open(IO_ReadOnly)) {
    cerr << "error" << endl;
    cerr << "Couldn't open import file " << filename << endl;
    return 0;
  }
  
  QCString str;
  
  QTextStream t(&f);

  while (!t.eof())
    str += t.readLine() + '\n';
  
  cerr << "done" << endl;

  cerr << "Filtering ... " << endl;
  // Filter !
  Q_UINT32 imported = filter(str, this);
  
  cerr << "Filtering done" << endl;
  
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
  cerr << "Closing backend library ... ";
  ::dlclose(handle_);
  cerr << "done" << endl;
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
KAB::AddressBook::write(Entity * e)
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

  void
KAB::AddressBook::write()
{
  return;
  QStrList idList;
  
  allKeys_(idList);
  
  QStrListIterator it(idList);

  for (; it.current(); ++it) {
    Entity * e(entity(it.current()));
    if (e == 0) continue;
    if (e->isDirty())
      e->write(e->id().ascii(), &write_);
  }
}

  QStrList
KAB::AddressBook::topLevelGroups()
{
  QStrList out;
  QStrList l(keysOfType(EntityTypeGroup));
  
  QStrListIterator it(l);
  
  QList<Group> groups;
  
  for (; it.current(); ++it) {
    Entity * e = entity(it.current());
    if (e == 0) continue;
    Group * g = (Group *)e;
    groups.append(g);
  }
  
  // For each group
  QListIterator<Group> git(groups);
  
  for (; git.current(); ++git) {
    
    // For each group
    QListIterator<Group> git2(groups);
    
    bool found = false;
    
    for (; git2.current(); ++git2) {
      
      // Does the current group's id appear in the subGroupList of the
      // other group ?
      if (git2.current()->subGroupList().contains(git.current()->id())) {
        found = true;
        break;
      }
    }
    if (!found)
      out.append(git.current()->id());
  }
  
  return out;
}
    
  KAB::Person *
KAB::AddressBook::person(const QString & key)
{ Entity * e = entity(key); return e == 0 ? 0 : (Person *)e;    }

  KAB::Location *
KAB::AddressBook::location(const QString & key)
{ Entity * e = entity(key); return e == 0 ? 0 : (Location *)e;  }

  KAB::Member *
KAB::AddressBook::member(const QString & key)
{ Entity * e = entity(key); return e == 0 ? 0 : (Member *)e;    }

  KAB::Group *
KAB::AddressBook::group(const QString & key)
{ Entity * e = entity(key); return e == 0 ? 0 : (Group *)e;     }
 
