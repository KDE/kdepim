#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qcstring.h>
#include <qdatastream.h>

#include <Entry.h>
#include <Field.h>
#include <AddressBook.h>

#include <VCardEntity.h>
#include <ldif.h>

#include <dlfcn.h>

using namespace KAB;

AddressBook::AddressBook()
{
//  entryDict_.setAutoDelete(true);
}

AddressBook::~AddressBook()
{
  // Empty.
}

AddressBook::AddressBook(const AddressBook & ab)
  :  entryDict_    (ab.entryDict_)
{
//  entryDict_.setAutoDelete(true);
}

  AddressBook &
AddressBook::operator = (const AddressBook & ab)
{
  entryDict_ = ab.entryDict_;
  return *this;
}

  Entry *
AddressBook::entry(const QString & name)
{
  return entryDict_[name];
}

  EntryList
AddressBook::entries(const QRegExp & e)
{
  EntryList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it)
    if (e.match(it.current()->name()) != -1)
      l.append(*it.current());
  
  return l;
}

  FieldList
AddressBook::fields(const QString & s)
{
  FieldList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it) {
  
    FieldList f(it.current()->fields(s));
  
    for(
      FieldListConstIterator fit(f.begin());
      fit != f.end();
      ++fit)
      l.append(*fit);
  }

  return l;
}

  FieldList
AddressBook::fields(const QRegExp & e)
{
  FieldList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it) {
  
    FieldList f(it.current()->fields(e));
  
    for (
      FieldListConstIterator fit(f.begin());
      fit != f.end();
      ++fit)
      l.append(*fit);
  }
  
  return l;
}

  FieldList
AddressBook::fieldsWithValueType(ValueType t)
{
  FieldList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it) {
   
    FieldList f(it.current()->fieldsWithValueType(t));
  
    for (
      FieldListConstIterator fit(f.begin());
      fit != f.end();
      ++fit)
      l.append(*fit);
  }
  
  return l;
}  
  
  FieldList
AddressBook::fieldsWithValueType(const QString & valueType)
{
  FieldList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it) {
    
    FieldList f(it.current()->fieldsWithValueType(valueType));
    
    for (
      FieldListConstIterator fit(f.begin());
      fit != f.end();
      ++fit)
      l.append(*fit);
  }
  
  return l;
}

  FieldList
AddressBook::fieldsWithExtensionValueType()
{
  return fieldsWithValueType(XValue);
}

  FieldList
AddressBook::fieldsWithStandardValueType()
{
  FieldList l;
  
  for (EntryDictIterator it(entryDict_); it.current(); ++it) {
   
    FieldList f(it.current()->fieldsWithStandardValueType());
  
    for (
      FieldListConstIterator fit(f.begin());
      fit != f.end();
      ++fit)
      l.append(*fit);
  }
  
  return l;
}

  bool
AddressBook::add(const Entry & e)
{
  if (e.name().isEmpty()) {
    kabDebug("Name is null, not adding!");
    return false;
  }
  
  entryDict_.insert(e.name(), new Entry(e));
  
  return true;
}

  bool
AddressBook::remove(const QString & key)
{
  bool retval = entryDict_.remove(key);

  return retval;
}

  bool
AddressBook::update(const Entry & e)
{
  if (!e.name()) return false;
  
  entryDict_.replace(e.name(), new Entry(e));
  
  return true;
}

  Q_UINT32
AddressBook::import(const QString & format, const QString & filename)
{
  // Try to dynamically load a filter plugin for the specified type.
    
  QCString filterFilename =
    QCString("lib" + QCString(format.lower().ascii()) + "filter.so");
    
  FilterHandle handle(::dlopen(filterFilename, RTLD_LAZY));
    
  if (!handle) {
    kabDebug("Couldn't open filter plugin " + filterFilename);
    kabDebug(QString(::dlerror()));
    return 0;
  }
    
  Filter filter((Filter)::dlsym(handle, "import"));
    
  if (!filter) {
    kabDebug("Unable to init filter plugin for " + format);
    kabDebug(QString(::dlerror()));
    ::dlclose(handle);
    return 0;
  }
    
  // Now load the data we'll be importing.

  QFile f(filename);
  
  if (!f.open(IO_ReadOnly)) {
    kabDebug("Couldn't open import file " + filename);
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

  QDataStream &
KAB::operator >> (QDataStream & s, AddressBook & a)
{
  EntryList entryList;
  
  s >> entryList;
  
  for (
    EntryListConstIterator it(entryList.begin());
    it != entryList.end();
    ++it)
    a.add(*it);

  return s;
}

  QDataStream &
KAB::operator << (QDataStream & s, const AddressBook & a)
{
  EntryList entryList;
  
  for (
    EntryDictIterator it(a.entryDict_);
    it.current();
    ++it)
    entryList.append(*it.current());
  
  s << entryList;
  
  return s;
}

  bool
AddressBook::save(const QString & filename)
{
  QFile f(filename);
  
  if (!f.open(IO_WriteOnly)) {
    kabDebug("Failed to save to file " + filename);
    return false;
  }
  
  QDataStream d(&f);

  d << *this;
  
  return true;
}

  bool
AddressBook::load(const QString & filename)
{
  QFile f(filename);
  
  if (!f.open(IO_ReadOnly)) {
    kabDebug("Failed to load from file " + filename);
    return false;
  }
  
  QDataStream d(&f);
  
  entryDict_.clear();

  operator >> (d, *this);
  
  return true;
}

// vim:ts=2:sw=2:tw=78:

