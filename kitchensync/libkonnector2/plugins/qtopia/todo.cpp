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

#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include <calendarsyncee.h>
#include <idhelper.h>
#include <libkcal/calendarlocal.h>

#include "device.h"
#include "todo.h"

using namespace OpieHelper;


ToDo::ToDo( CategoryEdit* edit,
            KSync::KonnectorUIDHelper* helper,
            const QString &tz,
            Device* dev)
    : Base( edit,  helper,  tz, dev )
{
}
ToDo::~ToDo(){
}

KCal::Todo* ToDo::dom2todo( QDomElement e, ExtraMap& extra,const QStringList& lst ) {
    QString dummy;
    int Int;
    KCal::Todo* todo = new KCal::Todo();
    QStringList list = QStringList::split(";",  e.attribute("Categories") );
    QStringList categories;

    QString cat;
    for ( uint i = 0; i < list.count(); i++ ) {
        cat = m_edit->categoryById( list[i], "Todo List");
	/* only if cat is not empty and not already added */
        if (!cat.isEmpty() && !categories.contains( cat) )
            categories.append(cat );
    }
    if (!categories.isEmpty() ) {
        todo->setCategories( categories );
    }

    todo->setDescription(e.attribute("Description" ) );
    todo->setSummary( e.attribute("Summary") ); //opie only
    if ( ( device() && device()->distribution() == Device::Zaurus ) || todo->summary().isEmpty() )
        todo->setSummary( e.attribute("Description").stripWhiteSpace().left(20).simplifyWhiteSpace() );

    setUid(todo,  e.attribute("Uid")  );

    dummy = e.attribute("Completed");

    QDate comp = e.hasAttribute( "CompletedDate" ) ?
                 stringToDate( e.attribute( "CompletedDate" ) ):
                 QDate();

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


    /* !0 */
    if ( !Int ) {
        todo->setCompleted( false );

        /*
         * libkcal wants to be too smart again
         * 100% percent done but not completed
         * will be marked as completed...
         * So let us save the completion status
         * and apply it on writeback
         */
        int prog =  e.attribute("Progress").toInt();
        todo->setPercentComplete( prog );
        extra.add("todo", "CompletionItem", e.attribute("Uid"),
                  new TodoExtraItem( Int, prog ) );
    }else {
      if ( comp.isValid() )
        todo->setCompleted( comp );
      else
        todo->setCompleted(true );
    }




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

        /*
         * libkcal does not set HasDueDate TRUE
         * if we supply a due date
         */
        todo->setHasDueDate( true );
    }else{
        todo->setHasDueDate( false );
    }

    if ( e.hasAttribute( "StartDate" ) ) {
      todo->setHasStartDate( true );
      todo->setDtStart( stringToDate(e.attribute( "StartDate")) );
    }

    // time to add extra not used attributes
    extra.add("todo", e.attribute("Uid"),  e.attributes(), lst );

    return todo;
}

bool ToDo::toKDE( const QString &fileName, ExtraMap& map, KSync::CalendarSyncee *syncee )
{
  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    return false;
  }

  QDomDocument doc( "mydocument" );
  if ( !doc.setContent( &file ) ) {
    return false;
  }

  QStringList attr = supportedAttributes();
  QDomElement docElem = doc.documentElement();
  KCal::Todo *todo;
  QDomNode n = docElem.firstChild();
  while ( !n.isNull() ) {
    QDomElement e = n.toElement();
    if ( !e.isNull() ) {
      if ( e.tagName() == "Task" ) {
        todo = dom2todo( e, map,attr );
        KSync::CalendarSyncEntry* entry;
        entry = new KSync::CalendarSyncEntry( todo, syncee );
        syncee->addEntry( entry );
      }
    }

    n = n.nextSibling();
  }

  return true;
}

KTempFile* ToDo::fromKDE( KSync::CalendarSyncee* syncee, ExtraMap& map )
{
    // KDE ID clear bit first
    m_kde2opie.clear();
    Kontainer::ValueList newIds = syncee->ids( "TodoSyncEntry");
    for ( Kontainer::ValueList::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("TodoSyncEntry",  (*idIt).first,  (*idIt).second );
    }
    // update m_helper first;
    KTempFile* tmpFile = file();
    if (tmpFile->textStream() ) {
        // clear list
        KSync::CalendarSyncEntry* entry;
        QTextStream *stream = tmpFile->textStream();
        stream->setEncoding( QTextStream::UnicodeUTF8 );
        *stream << "<!DOCTYPE Tasks>" << endl;
        *stream << "<Tasks>" << endl;
        for ( entry = (KSync::CalendarSyncEntry*)syncee->firstEntry();
              entry != 0l;
              entry = (KSync::CalendarSyncEntry*)syncee->nextEntry() )
        {
            if ( entry->wasRemoved() )
                continue;

            KCal::Todo *todo = dynamic_cast<KCal::Todo*>( entry->incidence() );
            if ( !todo )
              continue;

            *stream << todo2String( todo, map ) << endl;
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

QString ToDo::todo2String( KCal::Todo* todo, ExtraMap& map )
{
  // id hacking We don't want to have the ids growing and growing
  // when an id is used again it will be put to the used list and after done
  // with syncing we will replace the former
  QString uid = konnectorId("TodoSyncEntry", todo->uid() );

  QString text;
    text.append("<Task ");
    QStringList list = todo->categories();
    text.append( appendText( "Categories=\"" +
                             categoriesToNumber( list ) + "\" ",
                             "Categories=\"\" " ) );

    {
      bool comp     = todo->isCompleted();
      int completed = todo->percentComplete();

      if ( completed == 100 ) {
        TodoExtraItem *item = static_cast<TodoExtraItem*>(
                              map.item( "todo", "CompletionItem",
                                        uid ) );
        comp = (item &&item->completion==100) ? item->completed : comp;
      }

      text.append( "Completed=\""+QString::number(comp) + "\" " );
      text.append( "Progress=\"" + QString::number(completed) + "\" ");

      if ( comp && todo->hasCompletedDate() )
        text.append( "CompletedDate=\"" +
                     dateToString( todo->completed().date() ) + "\" ");
    }

    /* if it is not a Stock Zaurus we will right the summary */
    if ( device() && device()->distribution() != Device::Zaurus )
        text.append( appendText( "Summary=\"" +escape( todo->summary() )+"\" ",
                                 "Summary=\"\" "));

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

    /* if Opie let's write the description right away
     * else we need to find out if description is empty and then
     * fallback to the summary if both are empty you're lost!
     **/
    if ( device() && device()->distribution() != Device::Zaurus )
        text.append( appendText("Description=\"" +escape( todo->description() ) + "\" ",
                                "Description=\"\" " ) );
    else{
        QString desc = todo->description().isEmpty() ?
                       todo->summary() : todo->description();
        text.append( appendText("Description=\"" +escape( desc ) + "\" ",
                                "Description=\"\" " ));
    }

    if ( todo->hasStartDate() )
      text.append( "StartDate=\"" + dateToString( todo->dtStart().date() )
                   + "\" " );


    text.append("Uid=\"" +uid + "\" "  );

    /* add custom entries */
    text.append( map.toString("todo", uid ) );

    text.append(" />");
    return text;
}

QStringList ToDo::supportedAttributes() {
    QStringList lst;
    lst << "Categories";
    lst << "Completed";
    lst << "Progress";
    lst << "Summary";
    lst << "HasDate";
    lst << "DateDay";
    lst << "DateMonth";
    lst << "DateYear";
    lst << "Priority";
    lst << "Description";
    lst << "Uid";
    lst << "StartDate";
    lst << "CompletedDate";

    return lst;
}

QString  ToDo::dateToString( const QDate& date  ) {
  return date.toString( "yyyyMMdd" );
}

QDate    ToDo::stringToDate( const QString& s ) {
  // From OPimDateConversion
  // Read ISO-Format (YYYYMMDD)
  int year = s.mid( 0, 4 ).toInt();
  int month = s.mid( 4, 2 ).toInt();
  int day = s.mid( 6, 2 ).toInt();

  QDate date;
  date.setYMD( year, month, day );

  return date;
}
