
#include <kdebug.h>
#include <qregexp.h>
#include <qdom.h>
#include <qfile.h>
#include <kapplication.h>

#include <kstaticdeleter.h>

#include <kabc/resourcefile.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

#include <libkcal/todo.h>

#include "opiehelper.h"

namespace {
QString categoryById(const QString &id, const QString &app, QValueList<OpieCategories> cate )
{
  QValueList<OpieCategories>::Iterator it;
  QString category;
  for( it = cate.begin(); it != cate.end(); ++it ){
    kdDebug(5202) << "it :" << (*it).id() << "id:" << id << "ende"<<endl;
    if( id.stripWhiteSpace() == (*it).id().stripWhiteSpace() ){
      //if( app == (*it).app() ){
      kdDebug(5202) << "found category" << endl;
      category = (*it).name();
      break;
      //}
    }else {
      kdDebug(5202) << "not equal " << endl;
    }
    }
  kdDebug(5202) << "CategoryById: " << category << endl;
  return category;
};
  void dump(const KABC::Addressee &test ){
    kdDebug(5202) << "Addressee" << endl;
    kdDebug(5202) << "Name " << test.name() << endl;
    kdDebug(5202) << "UID" << test.uid() << endl;
    kdDebug(5202) << "LastName" << test.familyName() << endl;
  }

};

OpieHelperClass* OpieHelperClass::s_Self = 0;
KStaticDeleter<OpieHelperClass> opiehelpersd;

OpieHelperClass* OpieHelperClass::self()
{
  if( !s_Self )
    s_Self = opiehelpersd.setObject( new OpieHelperClass() );

  return s_Self;
};

void OpieHelperClass::toOpieDesktopEntry( const QString &str, QPtrList<KSyncEntry> *entry, OpieHelper::CategoryEdit *edit )
{
    QString string ( str );
    string.remove(0, 35 );
    string.replace(QRegExp("&amp;"), "&" );
    string.replace(QRegExp("&0x20;"), " ");
    string.replace(QRegExp("&0x0d;"), "\n");
    string.replace(QRegExp("&0x0a;"), "\r");
    string.replace(QRegExp("\r\n"), "\n" ); // hell we're on unix
    kdDebug(5202) << string << endl;
    if(!string.contains("[Desktop Entry]")  )
	return;
    QStringList list = QStringList::split('\n', string );
    QString name, category, type, fileName, size;
    QStringList::Iterator it;
    it = list.begin(); // desktopentry;

    list.remove( it );
    // yuyuyi
    OpieDesktopSyncEntry *entr;
    for( it = list.begin(); it != list.end(); ++it ){
	QString con( (*it) );
	con = con.stripWhiteSpace();
	kdDebug(5202) << "CurrentLine " << con << endl;
	if( con.startsWith("Categories = " ) ){ // FIXME for multiple Categories they're separated by a ;
	    con = con.remove(0, 13 );
	    con = con.remove( con.length() -1, 1 );
	    category = edit->categoryById( con, "Document View" );
	}else if(con.startsWith("Name = " ) ){
	    con = con.remove(0, 7);
	    name = con.stripWhiteSpace();
	}else if(con.startsWith("Type = " ) ) {
	    con = con.remove(0, 7 );
	    type = con.stripWhiteSpace();
	}else if(con.startsWith("File = " ) ){
	    con = con.remove(0, 7);
	    fileName = con.stripWhiteSpace();
	}else if(con.startsWith("Size = ") ){
	    con = con.remove(0, 7);
	    size = con.stripWhiteSpace();
	}
	if( (*it).stripWhiteSpace() == "[Desktop Entry]" ){ // ok next entry starts
	    kdDebug(5202) << "File:" << fileName << ":Name:" << name << ":Type:" << type <<  ":Size:" << size << endl;
	    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
	    entry->append( entr );
	}
    }
//ok here we got one more entry I guess I'm wrong here
    kdDebug(5202) << "File:" << fileName << ":Name:" << name << ":Type:" << type <<  ":Size:" << size << endl;
    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
    entry->append( entr );
}

 void OpieHelperClass::toCalendar(const QString &timestamp, const QString &todo, const QString &calendar, QPtrList<KSyncEntry> *list, OpieHelper::CategoryEdit *edit )
{
  KAlendarSyncEntry *entry = new KAlendarSyncEntry();
  list->append( entry );
  KCal::CalendarLocal *cal = new KCal::CalendarLocal();
  entry->setCalendar( cal );
  QFile file( todo );
  if( !file.open(IO_ReadOnly ) ){
    return;
  }
  QDomDocument doc("mydocument" );
  if( !doc.setContent( &file ) ){
    file.close();
  }else{
    QDomElement docElem = doc.documentElement( );
    kdDebug(5202 ) << docElem.tagName() ;
    QDomNode n =  docElem.firstChild();
    KCal::Todo *todo;
    QString dummy;
    int Int;
    bool ok;
    while(!n.isNull() ){
      QDomElement e =n.toElement();
      if(!e.isNull() ){
	kdDebug(5202) << e.tagName() << endl;
	if(e.tagName() == "Task" ){
	  todo = new KCal::Todo();
	  QStringList list = QStringList::split(";", e.attribute("Categories") );
	  QStringList categories;
	  for(int i=0; i< list.count(); i++ ){
	    categories.append( edit->categoryById(list[i], "Todo List"  ) );
	  };
	  if( !categories.isEmpty() ){
	    todo->setCategories( categories );
	  }
	  todo->setDescription(e.attribute("Description" ) );
	  todo->setSummary( e.attribute("Description").left(15) );
	  todo->setUid(e.attribute("Uid")  );
	  dummy = e.attribute("Completed");
	  Int = dummy.toInt(&ok);
	  if(ok ){
	    bool status = Int;
	    if( status ){
	      todo->setCompleted(true);
	    }else
	      todo->setCompleted( false );
	  }else
	    todo->setCompleted( false );
	  dummy = e.attribute("Priority" );
	  todo->setPriority(dummy.toInt(&ok )  );
	  dummy = e.attribute("HasDate" );
	  bool status = dummy.toInt(&ok );
	  if(status){
	    todo->setHasDueDate(true );
	    QDateTime time = QDateTime::currentDateTime();
	    QDate date;
	    dummy = e.attribute("DateDay" );
	    int day= dummy.toInt(&ok );
	    int month = e.attribute("DateMonth").toInt(&ok );
	    int year = e.attribute("DateYear").toInt(&ok );
	    date.setYMD(year, month, day);
	    time.setDate( date );
	    todo->setDtDue( time );
	  }else{
	    todo->setHasDueDate( false );
	  }
	  cal->addTodo( todo );
	}
      }
      n = n.nextSibling();
    };
  }
  // now parese the files :(
    //return entry;
  QFile file2( todo );
  if( !file2.open(IO_ReadOnly ) ){
    return;
  }
  QDomDocument doc2("mydocument" );
  if( !doc2.setContent( &file2 ) ){
    file2.close();
  }else{
    // start reading
  }
}
void OpieHelperClass::toAddressbook(const QString &timeStamp, const QString &fileName, QPtrList<KSyncEntry> *list, OpieHelper::CategoryEdit *edit)
{
    KAddressbookSyncEntry *entry = new KAddressbookSyncEntry();
    list->append( entry );
    //return entry;
    QFile file( fileName );
    if( !file.open(IO_ReadOnly ) ){
	return;
    }
    QDomDocument doc("mydocument" );
    if( !doc.setContent( &file ) ){
	file.close();
	return;
    }

    entry->setId( kapp->randomString(8) );

    KABC::AddressBook *abook = new KABC::AddressBook( );
    entry->setAddressbook( abook );
    list->append( entry );


    QDomElement docElem = doc.documentElement( );
    QDomNode n =  docElem.firstChild();
    while(!n.isNull() ){
	QDomElement e = n.toElement();
	if(!e.isNull() ){
	    kdDebug(5202) << e.tagName() << endl;
	    if( e.tagName() == QString::fromLatin1("Contacts" ) ){ // we're looking for them
		QDomNode no = e.firstChild();
		while(!no.isNull() ){
		    QDomElement el = no.toElement();
		    if(!el.isNull() ){
			kdDebug(5202) << "Contacts: " << el.tagName() << endl;
			KABC::Addressee adr;
			adr.setUid(el.attribute("Uid" ) );
			adr.setFamilyName(el.attribute("LastName" ) );
			adr.setGivenName(el.attribute("FirstName" ) );
			adr.setAdditionalName(el.attribute("MiddleName" )  );
			adr.setSuffix(el.attribute("Suffix") );
			adr.setNickName(el.attribute("Nickname" ) );
			adr.setBirthday( QDate::fromString(el.attribute("Birthday")  ) );
			adr.setRole(el.attribute("JobTitle" ) );
			// inside into custom
			adr.setSortString( el.attribute("FileAs" ) );
			el.attribute("Department");
			adr.setOrganization( el.attribute("Company") );
			KABC::PhoneNumber businessPhoneNum(el.attribute("BusinessPhone"),
							   KABC::PhoneNumber::Work   );
			KABC::PhoneNumber businessFaxNum ( el.attribute("BusinessFax"),
							   KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax  );
			KABC::PhoneNumber businessMobile ( el.attribute("BusinessMobile"),
							   KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
			KABC::PhoneNumber businessPager( el.attribute("BusinessPager"),
							 KABC::PhoneNumber::Work | KABC::PhoneNumber::Pager);
			adr.insertPhoneNumber( businessPhoneNum );
			adr.insertPhoneNumber( businessFaxNum );
			adr.insertPhoneNumber( businessMobile );
			adr.insertPhoneNumber( businessPager  );
			adr.insertEmail( el.attribute("Emails") );

			KABC::PhoneNumber homePhoneNum( el.attribute("HomePhone"),
							KABC::PhoneNumber::Home );

			KABC::PhoneNumber homeFax (el.attribute("HomeFax"),
						   KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );

			KABC::PhoneNumber homeMobile( el.attribute("HomeMobile"),
						     KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
			adr.insertPhoneNumber(homePhoneNum );
			adr.insertPhoneNumber(homeFax );
			adr.insertPhoneNumber(homeMobile );
			KABC::Address business( KABC::Address::Work );

			business.setStreet  ( el.attribute("BusinessStreet") );
			business.setLocality( el.attribute("BusinessCity"  ) );
			business.setRegion  ( el.attribute("BusinessState" ) );
			business.setPostalCode( el.attribute("BusinessZip")  );

			adr.insertAddress( business );

			KABC::Address home( KABC::Address::Home );
			home.setStreet( el.attribute("HomeStreet") );
			home.setLocality( el.attribute("HomeCity") );
			home.setRegion( el.attribute("HomeState") );
			home.setPostalCode( el.attribute("HomeZip") );
			adr.insertAddress( home );
			//el.attribute("Birthday");

			adr.setNickName( el.attribute("Nickname") );
			adr.setNote( el.attribute("Notes") );

			QStringList categories = QStringList::split(";", el.attribute("Categories" ) );
			for(int i=0; i < categories.count(); i++ ){
			  adr.setCategories(edit->categoryById(categories[i], "Contacts"  ) );
			}
			adr.insertCustom("opie", "HomeWebPage", el.attribute("HomeWebPage") );
			adr.insertCustom("opie", "Spouse", el.attribute("Spouse") );
			adr.insertCustom("opie", "Gender", el.attribute("Gender") );
			adr.insertCustom("opie", "Anniversary", el.attribute("Anniversary") );
			adr.insertCustom("opie", "Children", el.attribute("Children") );
			adr.insertCustom("opie", "Office", el.attribute("Office") );
			adr.insertCustom("opie", "Profession", el.attribute("Profession") );
			adr.insertCustom("opie", "Assistant", el.attribute("Assistant") );
			adr.insertCustom("opie", "Manager", el.attribute("Manager") );
			abook->insertAddressee(adr );
		    }
		    no = no.nextSibling();
		}
	    }
	}
	n = n.nextSibling();
    }
    /*    KABC::ResourceFile r( abook, "/home/ich/addressbook.vcf" );
    abook->addResource(&r );
    KABC::Ticket *t = abook->requestSaveTicket( &r );
    abook->save( t );*/
    kdDebug(5202) << "Dumped " << endl;
}


