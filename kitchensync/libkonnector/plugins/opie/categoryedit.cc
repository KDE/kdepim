
#include <time.h>

#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
//#include <qstring.h>
#include <qtextstream.h>


#include <kdebug.h>

#include "categoryedit.h"


using namespace OpieHelper;

CategoryEdit::CategoryEdit()
{

}
CategoryEdit::CategoryEdit(const QString &fileName)
{
    parse( fileName );
}
CategoryEdit::~CategoryEdit()
{

}
QByteArray CategoryEdit::file()const
{
    QByteArray array;
    QBuffer buffer( array );
    if ( buffer.open( IO_WriteOnly ) ) {
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<!DOCTYPE CategoryList>" << endl;
        stream << "<Categories>" << endl;
        for ( QValueList<OpieCategories>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it ) {
            stream << "<Category id=\""<< (*it).id() << "\" ";
            if ( !(*it).app().isEmpty() )
                stream << " app=\""<< (*it).app() <<  "\" ";
            stream << "name=\"" << (*it).name() << "\" ";
            stream << " />" << endl;
        }
        stream << "</Categories>" << endl;
        buffer.close();
    }
    return array;
}
int CategoryEdit::addCategory( const QString &name, int id )
{
    return addCategory( QString::null, name, id );
}
int CategoryEdit::addCategory( const QString &appName,  const QString &name,  int id )
{
    if ( id == 0 ) {
        // code from tt
        //generate uid
        int id = -1 * (int) ::time(NULL );
        while ( ids.contains( id ) ){
            id += -1;
            if ( id > 0 )
                id = -1;
        }
    }
    ids.insert( id,  TRUE );
    OpieCategories categories(QString::number(id),  name,  appName);
    m_categories.remove( categories);
    m_categories.append( categories);
    return id;
}
void CategoryEdit::parse( const QString &tempFile )
{
    clear();
    kdDebug(5202) << "parsing the categories" << endl;
    QDomDocument doc( "mydocument" );
    QFile f( tempFile );
    if ( !f.open( IO_ReadOnly ) ){
	kdDebug(5202) << "can not open " <<tempFile << endl;
	return;
    }
    if ( !doc.setContent( &f ) ) {
	kdDebug(5202) << "can not setContent" << endl;
	f.close();
	return;
    }
    f.close();
    // print out the element names of all elements that are a direct child
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
//    kdDebug(5202) << "NodeName: " << docElem.nodeName() << endl;
    if( docElem.nodeName() == QString::fromLatin1("Categories") ){
	//kdDebug(5202) << "Category" << endl;
	while( !n.isNull() ) {
	    QDomElement e = n.toElement(); // try to convert the node to an element.
	    if( !e.isNull() ) { // the node was really an element.
		//kdDebug(5202) << "tag name" << e.tagName() << endl;
		QString id = e.attribute("id" );
		QString app = e.attribute("app" );
		QString name = e.attribute("name");
		OpieCategories category( id, name, app );
		//kdDebug(5202) << "Cat " << id << " " << app << " " << name << endl;
		m_categories.append( category ); // cheater
	    }
	    n = n.nextSibling();
	}
    }


}
void CategoryEdit::clear()
{
    ids.clear();
    m_categories.clear();
}
QString CategoryEdit::categoryById( const QString &id,  const QString &app )const
{
    QValueList<OpieCategories>::ConstIterator it;
    QString category;
    QString fallback;
    for( it = m_categories.begin(); it != m_categories.end(); ++it ){
	kdDebug(5202) << "it :" << (*it).id() << "id:" << id << "ende"<<endl;
	if( id.stripWhiteSpace() == (*it).id().stripWhiteSpace() ){
	    if( app == (*it).app() ){
	    //kdDebug(5202) << "found category" << endl;
                category = (*it).name();
                break;
            }else{
                fallback = (*it).name();
            }
        }
    //kdDebug(5202) << "CategoryById: " << category << endl;
    }
    return category.isEmpty() ? fallback : category;
}
