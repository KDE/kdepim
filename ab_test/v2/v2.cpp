#include <iostream>
#include <qfile.h>

#include "KabEnum.h"
#include "KabAddressBook.h"
#include "KabComms.h"
#include "KabEmailAddress.h"
#include "KabEntity.h"
#include "KabEnum.h"
#include "KabField.h"
#include "KabGroup.h"
#include "KabLocation.h"
#include "KabMember.h"
#include "KabPerson.h"
#include "KabPersonalName.h"
#include "KabSubValue.h"
#include "KabTalkAddress.h"
#include "KabTelephone.h"

#include <iostream>

void create(KAB::AddressBook &);
void print(KAB::AddressBook &);
void testImport(KAB::AddressBook &);

main(int argc, char * argv[])
{
  if (argc != 4) {
    cerr << "Usage: v2 <create | print> <format> <url>" << endl;
    cerr << "Examples: v2 create gdbm file:/tmp/test.gdbm" << endl;
    cerr << "          v2 print  gdbm file:/tmp/test.gdbm" << endl;
    exit(1);
  }
  
  bool creating(strcmp(argv[1], "create") == 0);
  
  QString format(argv[2]);
  format = format.lower();

  KURL url(argv[3]);
  
  if (creating && QFile(url.path()).exists()) {
    cerr << "Database already exists. Remove it first !" << endl;
    exit(1);
  }
  
  KAB::AddressBook ab(format, url);
  if (!ab.usable()) {
    cerr << "Could not initialise addressbook for that format and location"
      << endl;
    exit(1);
  }
 
  if (creating)
    create(ab);
//    testImport(ab);
  else
    print(ab);
}

  void
create(KAB::AddressBook & ab)
{
  using namespace KAB;
  
  cerr << "Creating database" << endl;

 // Create some people.

  Person rik("It's Rik Hemsley");
  Person don("It's Don Sanders");
  
  // Create personal names for these people and assign them to the 'Person'
  // objects.
  
  PersonalName nameOfRik;
  
  nameOfRik.setFirstName ("Rik");
  nameOfRik.setLastName  ("Hemsley");
  
  QStringList otherNamesForRik;
  otherNamesForRik << "Mark" << "PretendName";
  
  nameOfRik.setOtherNames (otherNamesForRik);
  nameOfRik.setDisplayName("Rik Hemsley");
  nameOfRik.setPrefixes   ("Mr");
  nameOfRik.setSuffixes   ("BSc.");
  nameOfRik.setNickName   ("Rikkus");
  
  // Now, let's create an extension value for Rik.
  
  XValue x;
  x.setName("X-Height");
  x.setData(QCString("6'1\""));
  XValueList xl;
  xl.append(x);
  rik.setXValues(xl);

  PersonalName nameOfDon;

  nameOfDon.setFirstName  ("Don");
  nameOfDon.setLastName   ("Sanders");
  nameOfDon.setDisplayName("Don Sanders");
  nameOfDon.setNickName   ("Don old son");

  rik.setPersonalName(nameOfRik);
  don.setPersonalName(nameOfDon);
  
  // Add some more information to these persons.

  rik.setNotes  ("He's a smooth operator");
  rik.setGender (GenderMale);
  
  don.setNotes  ("KDE coder");
  don.setGender (GenderOther);
  
  // Create two groups. One is the KDE Organisation, the other
  // is a fictional company called [[without]] software.

  Group theKDEOrganisation("The KDE Organisation");
  Group withoutSoftware   ("[[without]] software");

  // Create a subgroup for each group.
  Group PIMTeam           ("The PIM team");
  Group withoutDirectors  ("Directors");
  
  // Add persons to subgroups.
  PIMTeam.addMember         (rik);
  PIMTeam.addMember         (don);
  withoutDirectors.addMember(rik);
  
  // Add the PIMTeam subgroup to the KDE group.
  theKDEOrganisation.addSubGroup(PIMTeam.id());
  
  // Add the withoutDirectors subgroup to the withoutSoftware group.
  withoutSoftware   .addSubGroup(withoutDirectors.id());
  
  ab.write(&theKDEOrganisation);
  ab.write(&withoutSoftware);
  ab.write(&PIMTeam);
  ab.write(&withoutDirectors);
  ab.write(&rik);
  ab.write(&don);
}
  void
print(KAB::AddressBook & ab)
{
  using namespace KAB;

  // First let's show the names of all entities.
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "List of all entities" << endl;
  cerr << "----------------------------------------------------------" << endl;

  QStrList allKeys = ab.allKeys();
  
  QStrListIterator allIt(allKeys);
 
  for (; allIt.current(); ++allIt) {
    Entity * e = ab.entity(allIt.current());
    cerr << "* " << e->name() << endl;
    delete e;
    e = 0;
  }
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "List of all toplevel groups and their members to one level" << endl;
  cerr << "----------------------------------------------------------" << endl;
  
  // Now let's try to do a little tree view of group, member.
  // To start with, we want all toplevel groups.

  QStrList topLevels = ab.topLevelGroups();
  
  QStrListIterator topLevelIt(topLevels);
  
  for (; topLevelIt.current(); ++topLevelIt) {
    
    Group * g = ab.group(topLevelIt.current());
    
    if (g == 0) continue;
    
    cerr << "* " << g->name() << endl;

    // Now get the list of all members of this group.    
    MemberRefList members = g->subGroupList();
    
    MemberRefList::ConstIterator mit(members.begin());
    
    for (; mit != members.end(); ++mit) {
      
      Member * m = ab.member(*mit);
      
      if (m == 0) continue;
      
      cerr << "|-> " << m->name() << endl;
      
      XValueList xValues = m->xValues();
      
      XValueList::ConstIterator xit(xValues.begin());
      
      for (; xit != xValues.end(); ++xit)
        cerr << "    XValue: " << (*xit).name() << ": " <<
          (*xit).data() << endl;
     
      delete m;
      m = 0;
    }
    
    delete g;
    g = 0;
  }
  cerr << "----------------------------------------------------------" << endl;
}

  void
testImport(KAB::AddressBook & ab)
{
  using namespace KAB;
  
  // Now let's get clever. We'll ask the addressbook to import
  // its own data. To specify the data type, we need only to pass a string,
  // which in this case is 'vcard'.
  
  QString format = "vcard";
  QString filename = "vcard1";
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "Attempting to load a plugin for '" << format <<
    "' and import file '" << filename << "'" << endl;
  cerr << "----------------------------------------------------------" << endl;
  
  Q_UINT32 imported = ab.import(format, filename);
 
  // 'imported' now contains the number of entities that were imported.
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "Import finished. Read " << imported << " entities." << endl;
  cerr << "----------------------------------------------------------" << endl;

  // Let's go through the list of entities and print some info about them.
  
  QStrList l = ab.allKeys();
  
  cerr << "There are " << l.count() << " keys in the db" << endl;
  
  QStrListIterator it(l);

  for (; it.current(); ++it) {
    
    cerr << "Looking for entity with key " << it.current() << endl;
    Entity * e = ab.entity(it.current());
    ASSERT(e != 0);
    
    cerr << "This entity is of type " << e->type() << endl;
    
    if (e->type() == EntityTypePerson) {
   
      // Name of the entity.
      
      Person * p = (Person *)e;
      
      cerr << "Person: " << p->name() << endl;
      
      // Now, assuming this is a person, we'll print their real name using
      // actual objects. 
      
      
      PersonalName pn = p->pname();
      
      cerr << "  Nickname     : " << pn.nickName() << endl;
      
      cerr << "  First name   : " << pn.firstName() << endl;
      
      // To get the middle name, we need to look at the list of 'other names'.
      // In our format, a person has an arbritary number of names, with some
      // of them 'special' to help us out. We have:
      //   DisplayName, Prefixes, FirstName, OtherNames,
      //   LastName, Suffixes, Nickname, XValues
      // This allows us to have as many names as we like. During that import
      // we have hopefully mapped the relevant fields to our own.
      
      cerr << "  Middle names :";
      QStringList otherNames = pn.otherNames();
      QStringList::ConstIterator otherit(otherNames.begin());
      for (; otherit != otherNames.end(); ++otherit)
        cerr << " " << (*otherit);
      cerr << endl;
      
      cerr << "  Last name    : " << pn.lastName() << endl;
      
      cerr << "  Email address: name: '" << p->contactInfo().email().name()
        << "' domain: '" << p->contactInfo().email().domain() << "'" << endl;
      
    } else if (e->type() == EntityTypeGroup) {
      
      Group * g = (Group *)e;
      
      cerr << "Toplevel group: " << g->name() << endl;

      // Now get the list of all members of this group and its subgroups.    
      MemberRefList members = g->subGroupList();
      
      MemberRefList::ConstIterator mit(members.begin());
      
      for (; mit != members.end(); ++mit) {
        
        Member * m = ab.member(*mit);

        if (m == 0) continue;
        
        cerr << "  Member: " << m->name() << endl;
        
        XValueList xValues = m->xValues();
        
        XValueList::ConstIterator xit(xValues.begin());
        
        for (; xit != xValues.end(); ++xit)
          cerr << "    XValue: " << (*xit).name() << ": " <<
            (*xit).data() << endl; 
      }
    }
    
    delete e;
    e = 0;
  }
}

