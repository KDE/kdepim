#include <iostream.h>

#include <qfile.h>
#include <Enum.h>
#include <AddressBook.h>
#include <Entry.h>
#include <Field.h>
#include <Value.h>

  KAB::AddressBook
createTest()
{
  using namespace KAB;
  
  AddressBook ab;
  
  Value v1("Mr",              Text);
  Value v2("Rik",             Text);
  Value v3("Mark",            Text);
  Value v4("Hemsley",         Text);
  Value v5("BSc",             Text);
  Value v6("rik@kde.org",     "email");
  Value v7("> average",       Integer);
  Value v8("15/1/1976",       Date);
  Value v9("+44 (0)191 blah","text");
  
  Field f1("Title",           v1);
  Field f2("Forename",        v2);
  Field f3("Middle name",     v3);
  Field f4("Surname",         v4);
  Field f5("Suffix",          v5);
  Field f6("Email",           v6);
  Field f7("Number of arms",  v7);
  Field f8("Date of birth",   v8);
  Field f9("Telephone",       v9);
  
  Entry e("Rik Mark Hemsley");

  e.add(f1);
  e.add(f2);
  e.add(f3);
  e.add(f4);
  e.add(f5);
  e.add(f6);
  e.add(f7);
  e.add(f8);
  e.add(f9);
  
  if (!ab.add(e)) {
    cerr << "Couldn't add entry" << endl;
    exit(1);
  }

  return ab;
}

  void
testFind(KAB::AddressBook & ab)
{
  using namespace KAB;

  Entry * ex = ab.entry("Rik Mark Hemsley");

  if (ex == 0) {
    cerr << "Couldn't find entry" << endl;
    exit(1);
  }
  
  FieldList fl;  
  fl = ex->fieldsWithValueType(Date);
  
  FieldListConstIterator it(fl.begin());
  
  for (; it != fl.end(); ++it) {
    
    int nsp = 20 - ((*it).name()).length();
    
    QString sp;
    
    for (int i = 0; i < nsp; i++)
      sp += ' ';
    
    cerr << (*it).name() << ":" << sp << (*it).data() << endl;
  }
}

  void
testImport(
  KAB::AddressBook & ab, const QString & format, const QString & filename)
{
  int imported = ab.import(format, filename);
  
  if (imported == 0) {
    cerr << "Failed to import from " << filename << endl;
    exit(1);
  }
  
  cerr << "Imported " << imported << " records from " <<
    filename << endl;
}

main(int argc, char * argv[])
{
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " <filename>" << endl;
    exit(1);
  }

  using namespace KAB;
  
  // Create an addressbook with code and save it.

  AddressBook ab = createTest();
  ab.save("kab_test_2");
  testFind(ab);
  
  // Create an addressbook by loading from a file.

  AddressBook ab2;
  ab2.load("kab_test_2");
  testFind(ab2);
  
  // Create an addressbook by importing from a file.
  // We don't actually have code to load a vCard. When we specify the format
  // as a string ("VCARD") the addressbook looks for a plugin that can handle
  // VCARD and if it finds it, loads it and asks it to do the import.

  AddressBook ab3;
  testImport(ab3, "VCARD", argv[1]);
  
  FieldList l = ab3.fieldsWithValueType(Any);
  
  for (FieldListConstIterator it(l.begin()); it != l.end(); ++it)
    cerr << (*it).name() << ": " << (*it).value().data() << endl;
}

// vim:ts=2:sw=2:tw=78:
