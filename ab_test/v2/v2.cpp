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

main()
{
  
  using namespace KAB;

  AddressBook ab;

  // Create some people.

  Person rik(ab, "It's Rik Hemsley");
  Person don(ab, "It's Don Sanders");
  
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
  char * mydata = "6'1\"";
  Data d;
  d.setRawData(mydata, strlen(mydata));
  x.setData(d);
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

  Group theKDEOrganisation(ab, "The KDE Organisation");
  Group withoutSoftware   (ab, "[[without]] software");

  // Create a subgroup for each group.
  Group PIMTeam           (theKDEOrganisation,  "The PIM team");
  Group withoutDirectors  (withoutSoftware,     "Directors");
  
  // Add persons to subgroups.
  PIMTeam.addMember         (rik);
  PIMTeam.addMember         (don);
  withoutDirectors.addMember(rik);
  
  // Add the PIMTeam subgroup to the KDE group.
  theKDEOrganisation.addSubGroup(PIMTeam.id());
  
  // Add the withoutDirectors subgroup to the withoutSoftware group.
  withoutSoftware   .addSubGroup(withoutDirectors.id());
  
  // Now let's look at our data.
  // First let's show the names of all entities.
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "List of all entities" << endl;
  cerr << "----------------------------------------------------------" << endl;

  EntityList allEntities = ab.allEntities();
  
  EntityListIterator it(allEntities);

  for (; it.current(); ++it)
    cerr << "Entity: " << (*it)->name() << endl;
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "List of all toplevel groups and their members (recursive)" << endl;
  cerr << "----------------------------------------------------------" << endl;
  
  // Now let's try to do a little tree view of group, member.
  // To start with, we want all toplevel groups.
  // Instead of traversing the tree of groups and subgroups ourselves
  // we'll simply ask each toplevel group for all members. The group
  // will recurse through its subgroups and get references to all members.
  // 
  // What we're getting here is the names of each toplevel group and then
  // the names of every member of that group and its subgroups.
  // 
  // Note: There isn't actually a recursive function in the code :)

  EntityList entities = ab.entitiesOfType("group");
  
  EntityListIterator eit(entities);
  
  for (; eit.current(); ++eit) {
    
    // Safe to cast here as we've been told that we're looking at groups only.
    // We can take a reference as we know the pointer eit.current() is valid.

    Group & d = *((Group *)eit.current());
    
    // We don't want non-toplevel groups.
    if (d.parent() != 0)
      continue;
    
    cerr << "Toplevel group: " << d.name() << endl;

    // Now get the list of all members of this group and its subgroups.    
    MemberRefList members = d.allMembers();
    
    MemberRefList::ConstIterator mit(members.begin());
    
    for (; mit != members.end(); ++mit) {
      
      Member & m = *((Member *)(ab.entity(*mit)));
      
      cerr << "  Member: " << m.name() << endl;
      
      XValueList xValues = m.xValues();
      
      XValueList::ConstIterator xit(xValues.begin());
      
      for (; xit != xValues.end(); ++xit)
        cerr << "    XValue: " << (*xit).name() << ": " <<
          (*xit).data() << endl; 
    }
  }
  
  // Now let's get clever. We'll create an addressbook and ask it to import
  // its own data. To specify the data type, we need only to pass a string,
  // which in this case is 'vcard'.
  
  QString format = "vcard";
  QString filename = "vcard1";
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "Attempting to load a plugin for '" << format <<
    "' and import file '" << filename << "'" << endl;
  cerr << "----------------------------------------------------------" << endl;
  
  AddressBook ab2;
  Q_UINT32 imported = ab2.import(format, filename);
 
  // 'imported' now contains the number of entities that were imported.
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "Import finished. Read " << imported << " entities." << endl;
  cerr << "Printing all entities" << endl;
  cerr << "----------------------------------------------------------" << endl;

  // Let's go through the list of entities and print some info about them.
  
  EntityList allEntities2 = ab2.allEntities();
  
  cerr << "There are " << allEntities2.count() << " entities" << endl;

  EntityListIterator it2(allEntities2);

  for (; it2.current(); ++it2) {
    
    // Name of the entity.
    
    cerr << "Entity: " << (*it2)->name() << endl;
    
    // Now, assuming this is a person, we'll print their real name using
    // actual objects. 
    
    cerr << "  Full name: " << endl;
    
    PersonalName pn = ((Person *)(*it2))->pname();
    
    cerr << "    First name  : " << pn.firstName() << endl;
    
    // To get the middle name, we need to look at the list of 'other names'.
    // In our format, a person has an arbritary number of names, with some
    // of them 'special' to help us out. We have:
    //   DisplayName, Prefixes, FirstName, OtherNames,
    //   LastName, Suffixes, Nickname, XValues
    // This allows us to have as many names as we like. During that import
    // we have hopefully mapped the relevant fields to our own.
    
    QStringList otherNames = pn.otherNames();
    QStringList::ConstIterator otherit(otherNames.begin());
    for (; otherit != otherNames.end(); ++otherit)
      cerr << "    Middle name : " << *otherit << endl;
    
    cerr << "    Last name   : " << pn.lastName() << endl;
  }
  
  cerr << "----------------------------------------------------------" << endl;
  cerr << "Well, that was fun. Coffee break ?" << endl;
  cerr << "----------------------------------------------------------" << endl;
}

