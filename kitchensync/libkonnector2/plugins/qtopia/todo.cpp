
#include <qbuffer.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <todosyncee.h>

#include <idhelper.h>

#include "device.h"
#include "opiecategories.h"
#include "todo.h"

using namespace OpieHelper;


ToDo::ToDo( CategoryEdit* edit,
            KSync::KonnectorUIDHelper* helper,
            const QString &tz,
            bool meta, Device* dev)
    : Base( edit,  helper,  tz,  meta, dev )
{
}
ToDo::~ToDo(){
}
KCal::Todo* ToDo::dom2todo( QDomElement e ) {
    QString dummy;
    int Int;
    KCal::Todo* todo = new KCal::Todo();
    QStringList list = QStringList::split(";",  e.attribute("Categories") );
    QStringList categories;

    for ( uint i = 0; i < list.count(); i++ ) {
        kdDebug(5227)<< list[i]
                     << " Category "
                     << m_edit->categoryById( list[i],  "Todo List")
                     << endl;
        categories.append(m_edit->categoryById(list[i], "Todo List") );
    }
    if (!categories.isEmpty() ) {
        kdDebug(5226) << "List " << list.join(";") << endl;
        kdDebug(5226) << "TransLated " << categories.join(";") << endl;
        todo->setCategories( categories );
    }

    todo->setDescription(e.attribute("Description" ) );
    todo->setSummary( e.attribute("Summary") ); //opie only
    if ( device() && device()->distribution() == Device::Zaurus )
        todo->setSummary( e.attribute("Description").stripWhiteSpace().left(20) );

    setUid(todo,  e.attribute("Uid")  );

    dummy = e.attribute("Completed");

    /*
     * if setCompleted is called
     * libkcal decides to put
     * percentage done to 100%
     * but if I put percentage
     * to say 50% it is not uncompleting the item
     * and if setCompleted( false ) is called
     * likcal sets percent completed to 0
     */
    Int = dummy.toInt();
    kdDebug(5227) << " Completed " << dummy << " " << Int << endl;

    /* !0 */
    if ( Int == 0) {
        kdDebug(5227) << "Calling not completed " << endl;
        todo->setCompleted( false );
        /*
         * libkcal wants to be too smart again
         * 100% percent done but not completed
         * will be marked as completed...
         */
        todo->setPercentComplete( e.attribute("Progress").toInt() );
    }else{
        kdDebug(5227) << "Todo is completed " << endl;
        todo->setCompleted(true );
    }



    kdDebug(5227) << "dummy completed " << todo->isCompleted()  << endl;

    dummy = e.attribute("Priority" );
    todo->setPriority(dummy.toInt( )  );
    dummy = e.attribute("HasDate" );
    bool status = dummy.toInt( );
    if(status){
        kdDebug(5227) << "Has Due Date " << endl;
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
        /*
         * libkcal does not set HasDueDate TRUE
         * if we supply a due date
         */
        todo->setHasDueDate( true );
    }else{
        todo->setHasDueDate( false );
    }
    return todo;
}
KSync::TodoSyncee* ToDo::toKDE( const QString &fileName )
{
    KSync::TodoSyncee* syncee = new KSync::TodoSyncee();
    syncee->setSource( "Opie");
    if( device() )
	syncee->setSupports( device()->supports( Device::Todolist ) );

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        return syncee;
    }
    QDomDocument doc("mydocument");
    if ( !doc.setContent( &file ) ){
        delete syncee;
        return 0;
    }

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
    return syncee;
}
KTempFile* ToDo::fromKDE( KSync::TodoSyncee* syncee )
{
    // KDE ID clear bit first
    m_kde2opie.clear();
    Kontainer::ValueList newIds = syncee->ids( "TodoSyncEntry");
    for ( Kontainer::ValueList::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("TodoSyncEntry",  (*idIt).first(),  (*idIt).second() );
    }
    // update m_helper first;
    KTempFile* tmpFile = file();
    if (tmpFile->textStream() ) {
        // clear list
        KSync::TodoSyncEntry* entry;
        QTextStream *stream = tmpFile->textStream();
        stream->setEncoding( QTextStream::UnicodeUTF8 );
        *stream << "<!DOCTYPE Tasks>" << endl;
        *stream << "<Tasks>" << endl;
        for ( entry = (KSync::TodoSyncEntry*)syncee->firstEntry();
              entry != 0l;
              entry = (KSync::TodoSyncEntry*)syncee->nextEntry() )
        {
            if ( entry->state() == KSync::SyncEntry::Removed )
                continue;
            *stream << todo2String( entry->todo() ) << endl;
        }
        *stream << "</Tasks>" << endl;
    }
    if (m_helper)
        m_helper->replaceIds( "TodoSyncEntry",  m_kde2opie );

    tmpFile->close();

    return tmpFile;
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
    kdDebug(5227) << " todo->isCompleted " << todo->isCompleted() << endl;
    text.append( "Completed=\""+QString::number( todo->isCompleted()) + "\" " );
    text.append( "Progress=\"" + QString::number( todo->percentComplete() ) + "\" ");
    text.append( "Summary=\"" + escape( todo->summary() ) + "\" ");
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
    text.append( "Description=\"" +escape( todo->description() ) + "\" " );

    // id hacking We don't want to have the ids growing and growing
    // when an id is used again it will be put to the used list and after done
    // with syncing we will replace the former
    text.append("Uid=\"" +konnectorId("TodoSyncEntry", todo->uid() )+ "\" "  );

    text.append(" />");
    return text;
}

