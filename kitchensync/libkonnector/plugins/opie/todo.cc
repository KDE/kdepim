
#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <kalendarsyncentry.h>
#include <idhelper.h>
#include "opiecategories.h"
#include "todo.h"

using namespace OpieHelper;

namespace {
    QString names2categories( const QStringList &list,  const QValueList<OpieCategories> &categories ) {
        QString dummy;
        QValueList<OpieCategories>::ConstIterator catIt;
        bool found = false;
        for ( QStringList::ConstIterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
            for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
                if ( (*catIt).name() == (*listIt) ) { // the same name
                    found= true;
                    dummy.append( (*catIt).id() + ";");
                }
            }
            //if ( !found )
            //  ; // generate a new category and add to the xml file FIMXME
        }
        if ( !dummy.isEmpty() )
            dummy.remove(dummy.length() -1,  1 ); //remove the last ;
        return dummy;
    }
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
            }
        }
        kdDebug() << "CategoryById: " << category << endl;
        return category;
    };
}

ToDo::ToDo( KonnectorUIDHelper* helper,  bool meta)
{
    m_helper = helper;
    m_meta = meta;
}
ToDo::~ToDo()
{

}
QPtrList<KCal::Todo> ToDo::toKDE( const QString &fileName, const QValueList<OpieCategories>&  cat)
{
    QPtrList<KCal::Todo> m_list;

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ){
            QDomElement docElem = doc.documentElement();
            KCal::Todo *todo;
            QDomNode n = docElem.firstChild();
            QString dummy;
            int Int;
            bool ok;
            while (!n.isNull() ) {
                QDomElement e = n.toElement();
                if (!e.isNull() ) {
                    if ( e.tagName() == "Task" ) {
                        todo = new KCal::Todo();
                        QStringList list = QStringList::split(";",  e.attribute("Categories") );
                        QStringList categories;
                        for ( uint i = 0; i < list.count(); i++ ) {
                            categories.append( categoryById(list[i],  QString::null,  cat ) );
                        }
                        if (!categories.isEmpty() ) {
                            todo->setCategories( categories );
                        }
                        todo->setDescription(e.attribute("Description" ) );
                        todo->setSummary( e.attribute("Description").left(15) );

                        setUid(todo,  e.attribute("Uid")  );
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
                        m_list.append( todo );
                    } // if name == "Task"
                } // e.isNull
                n = n.nextSibling();
            } // n.isNull
        } // setContent
    } // off open
    return m_list;
}
QByteArray ToDo::fromKDE(KAlendarSyncEntry* entry ,  const QValueList<OpieCategories> &categories)
{
    // update m_helper first;
    QByteArray array;
    QBuffer buffer( array );
    if ( buffer.open( IO_WriteOnly )) {
        // clear list
        QPtrList<KCal::Todo> list = entry->calendar()->getTodoList();
        KCal::Todo *todo;
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << "<!DOCTYPE Tasks>" << endl;
        stream << "<Tasks>" << endl;
        for ( todo = list.first(); todo != 0; todo = list.next() ) {
            stream << todo2String( todo, categories ) << endl;
        }
        stream << "</Tasks>" << endl;
        buffer.close();
    }
    return array;
}
void ToDo::setUid( KCal::Todo* todo,  const QString &uid )
{
    if ( m_helper == 0 )
        todo->setUid("Konnector-"+ uid);
    else{ // only if meta
        todo->setUid( m_helper->kdeId( "todo", uid,  "Konnector-"+uid) );
    }
}
// after getting a KSyncEntry back the dispatcher needs to update m_helper
// for us so we've our things installed
int ToDo::uid( const QString &udi )
{
    if ( m_helper == 0 ) {
//if ( udi.left(
    }else{

    }
}
QString ToDo::todo2String( KCal::Todo* todo,  const QValueList<OpieCategories> &categories )
{
    QString text;
    text.append("<Task ");
    QStringList list = todo->categories();
    text.append( "Categories=\"" +names2categories( list, categories ) + "\" " );
    text.append( "Completed=\""+QString::number( todo->isCompleted()) + "\" " );
    if ( todo->hasDueDate() ) {
        text.append( "HasDate=\"1\" ");
        QDateTime time = todo->dtDue();
        text.append( "DateDay=\"" +QString::number( time.date().day() ) + "\" ");
        text.append( "DateMonth=\"" + QString::number( time.date().month() ) + "\" " );
        text.append( "DateYear=\"" + QString::number( time.date().year() )+ "\" " );
    }else{
        text.append( "HasDate=\"0\" ");
    }
    text.append( "Priority=\"" + QString::number( todo->priority() ) +"\" " );
    text.append( "Description=\"" +todo->description() + "\" " );

    // id hacking We don't want to have the ids growing and growing
    // when an id is used again it will be put to the used list and after done
    // with syncing we will replace the former
    QString dummy = konnectorId("todo", todo->uid()  );
    text.append(" />");
    return text;
}

QString ToDo::konnectorId( const QString &appName,  const QString &uid )
{
    QString id;
    // Konnector-.length() ==  10
    if ( uid.startsWith( "Konnector-" ) ) { // not converted
        id =  uid.mid( 11 );
    }else if ( m_helper) {
        id =  m_helper->konnectorId( appName,  uid );
        //                        konnector kde
        kde2opie.append( Kontainer( id,     uid ) );
    }

    return id;
}
