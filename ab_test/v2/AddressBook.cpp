
#include <dlfcn.h>

#include <iostream>

#include <qfile.h>
#include <qtextstream.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qstring.h>

#include "KabAddressBook.h"

using namespace KAB;

int (*filterfn) (const char *, AddressBook *);
typedef   void *            FilterHandle;
typedef   typeof(filterfn)  Filter;

  Q_UINT32
AddressBook::import(const QString & format, const QString & filename)
{
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


