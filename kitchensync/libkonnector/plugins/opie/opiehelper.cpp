
#include <kdebug.h>
#include <qregexp.h>
#include <qdom.h>
#include <qfile.h>
#include <kapplication.h>

#include <kstaticdeleter.h>

#include "opiehelper.h"

namespace {
QString categoryById(const QString &id, const QString &app, QValueList<OpieCategories> cate )
{
    QValueList<OpieCategories>::Iterator it;
    QString category;
    for( it = cate.begin(); it != cate.end(); ++it ){
	kdDebug() << "it :" << (*it).id() << "id:" << id << "ende"<<endl; 
	if( id.stripWhiteSpace() == (*it).id().stripWhiteSpace() ){
	    //if( app == (*it).app() ){
	    kdDebug() << "found category" << endl;
	    category = (*it).name();
	    break;
		//}
	}else {
	    kdDebug() << "not equal " << endl;
	}
    }
    kdDebug() << "CategoryById: " << category << endl;
    return category;
};
  void dump(const KABC::Addressee &test ){
    kdDebug() << "Addressee" << endl;
    kdDebug() << "Name " << test.name() << endl;
    kdDebug() << "UID" << test.uid() << endl;
    kdDebug() << "LastName" << test.familyName() << endl;

  }

};

OpieHelper* OpieHelper::s_Self = 0;
KStaticDeleter<OpieHelper> opiehelpersd;

OpieHelper* OpieHelper::self()
{
  if( !s_Self )
    s_Self = opiehelpersd.setObject( new OpieHelper() );

  return s_Self;
};

void OpieHelper::toOpieDesktopEntry( const QString &str, QPtrList<KSyncEntry> *entry, const QValueList<OpieCategories> &cat )
{
    QString string ( str );
    string.remove(0, 35 );
    string.replace(QRegExp("&amp;"), "&" );
    string.replace(QRegExp("&0x20;"), " ");
    string.replace(QRegExp("&0x0d;"), "\n");
    string.replace(QRegExp("&0x0a;"), "\r");
    string.replace(QRegExp("\r\n"), "\n" ); // hell we're on unix
    kdDebug() << string << endl;
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
	kdDebug() << "CurrentLine " << con << endl;
	if( con.startsWith("Categories = " ) ){
	    con = con.remove(0, 13 );
	    con = con.remove( con.length() -1, 1 );
	    category = categoryById( con, "Document View", cat );
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
	    kdDebug() << "File:" << fileName << ":Name:" << name << ":Type:" << type <<  ":Size:" << size << endl;
	    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
	    entry->append( entr );
	}
    }
//ok here we got one more entry I guess I'm wrong here
    kdDebug() << "File:" << fileName << ":Name:" << name << ":Type:" << type <<  ":Size:" << size << endl;
    entr = new OpieDesktopSyncEntry(category, fileName, name, type, size );
    entry->append( entr );
}

 void OpieHelper::toCalendar(const QString &fileName, QPtrList<KSyncEntry> *list,const QValueList<OpieCategories> & )
{
    QPtrList<KAlendarSyncEntry> entry;
    //return entry;
}
void OpieHelper::toAddressbook(const QString &fileName, QPtrList<KSyncEntry> *list,const QValueList<OpieCategories> &)
{
    KAddressbookSyncEntry *entry = new KAddressbookSyncEntry();
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
	    kdDebug() << e.tagName() << endl;
	    if( e.tagName() == QString::fromLatin1("Contacts" ) ){ // we're looking for them
		QDomNode no = e.firstChild();
		while(!no.isNull() ){
		    QDomElement el = no.toElement();
		    if(!el.isNull() ){
			kdDebug() << "Contacts: " << el.tagName() << endl;
			KABC::Addressee adr;
			adr.setUid(el.attribute("Uid" ) );
			kdDebug()<< el.attribute("Uid" ) << endl;

			kdDebug() << el.attribute("LastName" ) << endl;
			adr.setFamilyName(el.attribute("LastName" ) );

			kdDebug() << el.attribute("FirstName" )<< endl;
			adr.setGivenName(el.attribute("FirstName" ) );

			kdDebug() << el.attribute("MiddleName" )<< endl;
			adr.setAdditionalName(el.attribute("MiddleName" )  );

			kdDebug() << el.attribute("Suffix") << endl;
			adr.setSuffix(el.attribute("Suffix") );

			kdDebug() << el.attribute("Nickname" ) << endl;
			adr.setNickName(el.attribute("Nickname" ) );

			kdDebug() <<el.attribute("Birthday")  << endl;
			adr.setBirthday( QDate::fromString(el.attribute("Birthday")  ) );
			
			kdDebug() << el.attribute("JobTitle" ) << endl;
			adr.setRole(el.attribute("JobTitle" ) );
			// inside into custom
			dump(adr);
			abook->insertAddressee(adr );


		    }
		    no = no.nextSibling();
		}
	    }
	}
	n = n.nextSibling();
    }
    abook->dump();
    kdDebug() << "Dumped " << endl;
}


