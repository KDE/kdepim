#include <dlfcn.h>

#include <iostream>

#include <qfile.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>

#include "KabAddressBook.h"
#include "KabGroup.h"
#include "KabEntity.h"

using namespace KAB;

  AddressBook *
AddressBook::create(
  const QString & name, const QString & format, const KURL & url)
{
  cerr << "Trying to create a new addressbook called " << name << endl;
  AddressBook * a;
  
  a = new AddressBook(name, format, url);
  
  if (!a->usable()) {
    cerr << "Addressbook " << name << " is unusable" << name << endl;
    delete a;
    a = 0;
  }

  return a;
}

  bool
AddressBook::_initBackend()
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
AddressBook::import(const QString & format, const QString & filename)
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

AddressBook::AddressBook(
  const QString & name, const QString & format, const KURL & url)
      :
        name_(name),
        format_ (format),
        url_    (url),
        usable_ (false)
{
  _initBackend();
}
        
AddressBook::AddressBook(const AddressBook & ab)
  : format_ (ab.format_),
    url_    (ab.url_),
    usable_ (false)
{
  _initBackend();
}

  AddressBook &
AddressBook::operator = (const KAB::AddressBook & ab)
{
  if (this == &ab) return *this;
  format_ = ab.format_;
  url_    = ab.url_;
  usable_ = false;
  _initBackend();
  return *this;
}

AddressBook::~AddressBook()
{
  cerr << "Closing backend library ... ";
  ::dlclose(handle_);
  cerr << "done" << endl;
}

  Entity *
AddressBook::entity(const QString & id)
{
  QCString key(id.ascii());
  
  EntityType t = keyToEntityType(key);
  
  Entity * e = 0;
  
  bool readok = false;
  
  switch (t) {
    
    case EntityTypeGroup:
      e = new Group;
      readok = ((Group *)e)->read(key, &read_);
      break;
    
    case EntityTypeEntity: default:
      e = new Entity;
      readok = e->read(key, &read_);
      break;
  }
  
  if (!readok) {
    delete e;
    e = 0;
  }
  
  return e;
}

  QStrList
AddressBook::keysOfType(EntityType t)
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
AddressBook::write(Entity * e)
{
  e->write(e->id().ascii(), &write_);
}

  bool
AddressBook::remove(const QString & key)
{
  return remove_(key.ascii());
}
  
  void
AddressBook::update(Entity * e)
{
  e->write(e->id().ascii(), &write_);
}

  QStrList
AddressBook::allKeys()
{
  QStrList l;
  allKeys_(l);
  return l;
}

  EntityType
AddressBook::keyToEntityType(const QCString & s)
{
  char c = s.at(s.length() - 1);
  EntityType t(EntityTypeEntity);
  
  switch (c) {
    case 'g':           t = EntityTypeGroup;    break;
    case 'e': default:  t = EntityTypeEntity;   break;
  }
  return t;
}

  Entity *
AddressBook::createEntityOfType(EntityType t)
{
  Entity * e (0);
  switch (t) {
    case EntityTypeGroup:           e = new Group;    break;
    case EntityTypeEntity: default: e = new Entity;  break;
  }
  return e;
}

  void
AddressBook::write()
{
  return;
}

  QStrList
AddressBook::topLevelGroups()
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
      if (git2.current()->members().contains(git.current()->id())) {
        found = true;
        break;
      }
    }
    if (!found)
      out.append(git.current()->id());
  }
  
  return out;
}
    
  Group *
AddressBook::group(const QString & key)
{
  Entity * e = entity(key);
  
  if (e == 0)
    return 0;
  
  if (e->type() != EntityTypeGroup)
    return 0;
  
  return (Group *)e;
}
 
