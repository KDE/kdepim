
#include <qbuffer.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <todosyncee.h>

#include <idhelper.h>

#include "opiecategories.h"
#include "todo.h"

using namespace OpieHelper;


ToDo::ToDo( CategoryEdit* edit,
            KSync::KonnectorUIDHelper* helper,
            const QString &tz,
            bool meta)
    : Base( edit,  helper,  tz,  meta )
{
}
ToDo::~ToDo()
{

}
KCal::Todo* ToDo::dom2todo( QDomElement e ) {
    QString dummy;
    int Int;
    KCal::Todo* todo = new KCal::Todo();
    QStringList list = QStringList::split(";",  e.attribute("Categories") );
    QStringList categories;

    for ( uint i = 0; i < list.count(); i++ ) {
        kdDebug(5202)<< list[i]
                     << " Category "
                     << m_edit->categoryById( list[i],  "Todo List")
                     << endl;
        categories.append(m_edit->categoryById(list[i], "Todo List") );
    }
    if (!categories.isEmpty() ) {
        todo->setCategories( categories );
    }

    todo->setDescription(e.attribute("Description" ) );
    todo->setSummary( e.attribute("Summary") ); //opie only

    setUid(todo,  e.attribute("Uid")  );

    dummy = e.attribute("Completed");
    Int = dummy.toInt();
    if ( Int )
        todo->setCompleted( true);
    else
        todo->setPercentComplete( e.attribute("Progress").toInt() );

    kdDebug() << "dummy completed " << dummy << " " << Int
              << endl;

    dummy = e.attribute("Priority" );
    todo->setPriority(dummy.toInt( )  );
    dummy = e.attribute("HasDate" );
    bool status = dummy.toInt( );
    if(status){
        todo->setHasDueDate(true );
        QDateTime time = QDateTime::currentDateTime();
        QDate date;
        dummy = e.attribute("DateDay" );
        int day= dummy.toInt( );
        int month = e.attribute("DateMonth").toInt( );
        int year = e.attribute("DateYear").toInt( );
        date.setYMD(year, month, day);
        time.setDate( date );
        todo->setDtDue( time );
    }else{
        todo->setHasDueDate( false );
    }
    return todo;
}
KSync::TodoSyncee* ToDo::toKDE( const QString &fileName )
{
    KSync::TodoSyncee* syncee = new KSync::TodoSyncee();

    QFile file( fileName );
    if ( file.open( IO_ReadOnly ) ) {
        QDomDocument doc("mydocument");
        if ( doc.setContent( &file ) ){
            QDomElement docElem = doc.documentElement();
            KCal::Todo *todo;
            QDomNode n = docElem.firstChild();
            while (!n.isNull() ) {
                QDomElement e = n.toElement();
                if (!e.isNull() ) {
                    if ( e.tagName() == "Task" ) {
                        todo = dom2todo( e );
                        KSync::TodoSyncEntry* entry;
                        entry = new KSync::TodoSyncEntry( todo );
                        syncee->addEntry( entry );
                    } // if name == "Task"
                } // e.isNull
                n = n.nextSibling();
            } // n.isNull
        } // setContent
    } // off open
    return syncee;
}
QByteArray ToDo::fromKDE( KSync::TodoSyncee* syncee )
{
    // KDE ID clear bit first
    m_kde2opie.clear();
    Kontainer::ValueList newIds = syncee->ids( "TodoSyncEntry");
    for ( Kontainer::ValueList::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("TodoSyncEntry",  (*idIt).first(),  (*idIt).second() );
    }
    // update m_helper first;
    QByteArray array;
    QBuffer buffer( array );
    if ( buffer.open( IO_WriteOnly )) {
        // clear list
        KSync::TodoSyncEntry* entry;
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << "<!DOCTYPE Tasks>" << endl;
        stream << "<Tasks>" << endl;
        for ( entry = (KSync::TodoSyncEntry*)syncee->firstEntry();
              entry != 0l;
              entry = (KSync::TodoSyncEntry*)syncee->nextEntry() )
        {
            if ( entry->state() == KSync::SyncEntry::Removed )
                continue;
            stream << todo2String( entry->todo() ) << endl;
        }
        stream << "</Tasks>" << endl;
        buffer.close();
    }
    if (m_helper)
        m_helper->replaceIds( "TodoSyncEntry",  m_kde2opie );
    return array;
}
void ToDo::setUid( KCal::Todo* todo,  const QString &uid )
{
    todo->setUid( kdeId( "TodoSyncEntry",  uid ) );
}

QString ToDo::todo2String( KCal::Todo* todo )
{
    QString text;
    text.append("<Task ");
    QStringList list = todo->categories();
    text.append( "Categories=\"" + categoriesToNumber( list ) + "\" " );
    text.append( "Completed=\""+QString::number( todo->isCompleted()) + "\" " );
    text.append( "Progress=\"" + QString::number( todo->percentComplete() ) + "\" ");
    text.append( "Summary=\"" + todo->summary() + "\" ");
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
    text.append("Uid=\"" +konnectorId("TodoSyncEntry", todo->uid() )+ "\" "  );

    text.append(" />");
    return text;
}

