
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


ToDo::ToDo( CategoryEdit* edit,
            KonnectorUIDHelper* helper,
            const QString &tz,
            bool meta)
    : Base( edit,  helper,  tz,  meta )
{
}
ToDo::~ToDo()
{

}
QPtrList<KCal::Todo> ToDo::toKDE( const QString &fileName )
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
                            kdDebug(5202)<< list[i] << " Category " << m_edit->categoryById( list[i],  "Todo List") << endl;
                            categories.append(m_edit->categoryById(list[i], "Todo List") );
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
QByteArray ToDo::fromKDE(KAlendarSyncEntry* entry )
{
    // KDE ID clear bit first
    m_kde2opie.clear();
    QValueList<Kontainer> newIds = entry->ids( "todo");
    for ( QValueList<Kontainer>::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("todo",  (*idIt).first(),  (*idIt).second() );
    }
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
            stream << todo2String( todo ) << endl;
        }
        stream << "</Tasks>" << endl;
        buffer.close();
    }
    m_helper->replaceIds( "todo",  m_kde2opie );
    return array;
}
void ToDo::setUid( KCal::Todo* todo,  const QString &uid )
{
    todo->setUid( kdeId( "todo",  uid ) );
}

QString ToDo::todo2String( KCal::Todo* todo )
{
    QString text;
    text.append("<Task ");
    QStringList list = todo->categories();
    text.append( "Categories=\"" + categoriesToNumber( list ) + "\" " );
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
    text.append("Uid=\"" +konnectorId("todo", todo->uid() )+ "\" "  );

    text.append(" />");
    return text;
}

