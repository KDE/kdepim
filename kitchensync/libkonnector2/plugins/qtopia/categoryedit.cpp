/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <time.h>

#include <qdom.h>
#include <qfile.h>
//#include <qstring.h>

#include <kconfig.h>

#include "helper.h"

#include "categoryedit.h"


using namespace OpieHelper;

CategoryEdit::CategoryEdit(){
}
CategoryEdit::CategoryEdit(const QString &fileName){
    parse( fileName );
}
CategoryEdit::~CategoryEdit(){
}
void CategoryEdit::save(const QString& fileName)const{
    QFile file( fileName );
    if ( file.open( IO_WriteOnly ) ) {
        QTextStream stream( &file );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<!DOCTYPE CategoryList>" << endl;
        stream << "<Categories>" << endl;
        for ( QValueList<OpieCategories>::ConstIterator it = m_categories.begin();
              it != m_categories.end(); ++it )
        {
            stream << "<Category id=\""<< escape( (*it).id() ) << "\" ";

            if ( !(*it).app().isEmpty() )
                stream << " app=\""<< escape( (*it).app() ) <<  "\" ";

            stream << "name=\"" << escape( (*it).name() ) << "\" ";
            stream << " />" << endl;
        }
        stream << "</Categories>" << endl;
        file.close();
    }
}
int CategoryEdit::addCategory( const QString &name, int id ){
    return addCategory( QString::null, name, id );
}
int CategoryEdit::addCategory( const QString &appName,  const QString &name,  int id ){
    if ( id == 0 ) {
        // code from tt
        //generate uid
        id = -1 * (int) ::time(NULL );
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
/*
 * we parse the simple Category File here
 * We also keep track of global Cats
 * and Of Organizer and Contact cats and then
 * we will add them to the kde side...
 */
void CategoryEdit::parse( const QString &tempFile ){
    clear();

    QDomDocument doc( "mydocument" );
    QFile f( tempFile );
    if ( !f.open( IO_ReadOnly ) )
	return;

    if ( !doc.setContent( &f ) ) {
	f.close();
	return;
    }
    f.close();

    QStringList global, contact, organizer;

    // print out the element names of all elements that are a direct child
    // of the outermost element.
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    if( docElem.nodeName() == QString::fromLatin1("Categories") ){
	while( !n.isNull() ) {
	    QDomElement e = n.toElement(); // try to convert the node to an element.
	    if( !e.isNull() ) { // the node was really an element.
		QString id = e.attribute("id" );
		QString app = e.attribute("app" );
		QString name = e.attribute("name");

                /*
                 * see where it belongs default to global
                 */
                if (app == QString::fromLatin1("Calendar") || app == QString::fromLatin1("Todo List") )
                    organizer.append( name );
                else if ( app == QString::fromLatin1("Contacts") )
                    contact.append( name );
                else
                    global.append( name );

		OpieCategories category( id, name, app );
		m_categories.append( category ); // cheater
	    }
	    n = n.nextSibling();
	}
    }
    updateKDE( "kaddressbookrc", global + contact );
    updateKDE( "korganizerrc", global + organizer );

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
	if( id.stripWhiteSpace() == (*it).id().stripWhiteSpace() ){
	    if( app == (*it).app() ){
                category = (*it).name();
                break;
            }else{
                fallback = (*it).name();
            }
        }
    }
    return category.isEmpty() ? fallback : category;
}
QStringList CategoryEdit::categoriesByIds( const QStringList& ids,
                                           const QString& app) {

    QStringList list;
    QStringList::ConstIterator it;
    QString temp;
    for ( it = ids.begin(); it != ids.end(); ++it ) {
        temp = categoryById( (*it), app );
        if (!temp.isEmpty() )
            list << temp;
    }

    return list;
}
void CategoryEdit::updateKDE( const QString& configFile,  const QStringList& cats ) {
    KConfig conf(configFile);
    conf.setGroup("General");
    QStringList avail = conf.readListEntry("Custom Categories");
    for (QStringList::ConstIterator it = cats.begin(); it !=  cats.end(); ++it ) {
        if (!avail.contains( (*it) ) )
            avail << (*it);
    }
    conf.writeEntry("Custom Categories", avail );
}
