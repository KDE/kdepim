#include "kincidenceformatter.h"
#include <kstaticdeleter.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#ifndef KORG_NOKABC
#include <kabc/stdaddressbook.h>
#define size count
#endif

KIncidenceFormatter* KIncidenceFormatter::mInstance = 0;
static KStaticDeleter<KIncidenceFormatter> insd;

QString KIncidenceFormatter::getFormattedText( Incidence * inc )
{

    mText = "";
    if ( inc->type() == "Event" )
        setEvent((Event *) inc );
    else if ( inc->type() == "Todo" )
        setTodo((Todo *) inc );
    else if ( inc->type() == "Journal" )
        mText = inc->description();
    return mText;
}

KIncidenceFormatter* KIncidenceFormatter::instance()
{
    if (!mInstance) {
    insd.setObject( mInstance, new KIncidenceFormatter());
  }
  return mInstance;
}
KIncidenceFormatter::~KIncidenceFormatter()
{
 
}
KIncidenceFormatter::KIncidenceFormatter()
{
    mColorMode = 0;
}
void KIncidenceFormatter::setEvent(Event *event)
{
    int mode = 0;
    mCurrentIncidence = event;
    bool shortDate = true;
    if ( mode == 0 ) {
        addTag("h3",event->summary());
    }
    else {
        if ( mColorMode == 1 ) {
            mText +="<font color=\"#00A000\">";
        }
        if ( mColorMode == 2 ) {
            mText +="<font color=\"#C00000\">";
        }
        // mText +="<font color=\"#F00000\">" + i18n("O-due!") + "</font>";
        if ( mode == 1 ) {
            addTag("h2",i18n( "Local: " ) +event->summary());
        } else {
            addTag("h2",i18n( "Remote: " ) +event->summary());
        } 
        addTag("h3",i18n( "Last modified: " ) + KGlobal::locale()->formatDateTime(event->lastModified(),shortDate, true ) );
        if ( mColorMode )
            mText += "</font>";
    } 
#if 0
    if (event->cancelled ()) {
        mText +="<font color=\"#B00000\">";
        addTag("i",i18n("This event has been cancelled!"));
        mText.append("<br>");
        mText += "</font>";
    }
#endif
    if (!event->location().isEmpty()) {
        addTag("b",i18n("Location: "));
        mText.append(event->location()+"<br>");
    }
    if (event->doesFloat()) {
        if (event->isMultiDay()) {
            mText.append(i18n("<p><b>From:</b> %1 </p><p><b>To:</b> %2</p>")
                         .arg(event->dtStartDateStr(shortDate))
                         .arg(event->dtEndDateStr(shortDate)));
        } else {
            mText.append(i18n("<p><b>On:</b> %1</p>").arg(event->dtStartDateStr( shortDate )));
        }
    } else {
        if (event->isMultiDay()) {
            mText.append(i18n("<p><b>From:</b> %1</p> ")
                         .arg(event->dtStartStr()));
            mText.append(i18n("<p><b>To:</b> %1</p>")
                         .arg(event->dtEndStr()));
        } else {
            mText.append(i18n("<p><b>On:</b> %1</p> ")
                         .arg(event->dtStartDateStr( shortDate )));
            mText.append(i18n("<p><b>From:</b> %1 <b>To:</b> %2</p>")
                         .arg(event->dtStartTimeStr())
                         .arg(event->dtEndTimeStr()));
        }
    }

    if (event->recurrence()->doesRecur()) {

        QString recurText = i18n("No");
        short recurs = event->recurrence()->doesRecur();  
        if ( recurs == Recurrence::rMinutely  )
            recurText = i18n("minutely");
        else if ( recurs == Recurrence::rHourly  )
            recurText = i18n("hourly");
        else if ( recurs == Recurrence::rDaily  )
            recurText = i18n("daily");
        else if ( recurs ==  Recurrence::rWeekly ) 
            recurText = i18n("weekly");
        else if ( recurs == Recurrence::rMonthlyPos  )
            recurText = i18n("monthly");
        else if ( recurs ==  Recurrence::rMonthlyDay )
            recurText = i18n("day-monthly");
        else if ( recurs ==  Recurrence::rYearlyMonth )
            recurText = i18n("month-yearly");
        else if ( recurs ==  Recurrence::rYearlyDay )
            recurText = i18n("day-yearly");
        else if ( recurs ==  Recurrence::rYearlyPos )
            recurText = i18n("position-yearly");
        addTag("p","<em>" + i18n("This is a %1 recurring event.").arg(recurText ) + "</em>");
        bool last;
        QDate start = QDate::currentDate();
        QDate next;
        next = event->recurrence()->getPreviousDate( start , &last );
        if ( !last ) {
            next = event->recurrence()->getNextDate( start.addDays( - 1 ) );
            addTag("p",i18n("Next recurrence is on: ")+ KGlobal::locale()->formatDate( next, shortDate ) );
            //addTag("p", KGlobal::locale()->formatDate( next, shortDate ));
        } else {
            addTag("p",i18n("<b>Last recurrence was on:</b>")  );
            addTag("p", KGlobal::locale()->formatDate( next, shortDate ));
        }
    }


    if (event->isAlarmEnabled()) {
        Alarm *alarm =event->alarms().first() ;
        QDateTime t = alarm->time();
        int min = t.secsTo( event->dtStart() )/60;
        QString s =i18n("(%1 min before)").arg( min );
        addTag("p",i18n("<b>Alarm on:  </b>")  + s  + ": "+KGlobal::locale()->formatDateTime( t, shortDate ));
        //addTag("p", KGlobal::locale()->formatDateTime( t, shortDate ));
        //addTag("p",s);
    }

    addTag("p",i18n("<b>Access: </b>") +event->secrecyStr() );
    // mText.append(event->secrecyStr()+"<br>");
    formatCategories(event);
    if (!event->description().isEmpty()) {
        addTag("p",i18n("<b>Details: </b>"));
        addTag("p",event->description());
    }


    formatReadOnly(event);
    formatAttendees(event);

 
}

void KIncidenceFormatter::setTodo(Todo *event )
{
    int mode = 0;
    mCurrentIncidence = event;
    bool shortDate = true;
    if (mode == 0 )
        addTag("h3",event->summary());
    else { 
        if ( mColorMode == 1 ) {
            mText +="<font color=\"#00A000\">";
        }
        if ( mColorMode == 2 ) {
            mText +="<font color=\"#B00000\">";
        }
        if ( mode == 1 ) {
            addTag("h2",i18n( "Local: " ) +event->summary());
        } else {
            addTag("h2",i18n( "Remote: " ) +event->summary());
        }
        addTag("h3",i18n( "Last modified: " ) + KGlobal::locale()->formatDateTime(event->lastModified(),shortDate, true ) );
        if ( mColorMode )
            mText += "</font>";
    } 
#if 0
    if (event->cancelled ()) {
        mText +="<font color=\"#B00000\">";
        addTag("i",i18n("This todo has been cancelled!"));
        mText.append("<br>");
        mText += "</font>";
    }
#endif    
    if (!event->location().isEmpty()) {
        addTag("b",i18n("Location: "));
        mText.append(event->location()+"<br>");
    }
    if (event->hasDueDate()) {
        mText.append(i18n("<p><b>Due on:</b> %1</p>").arg(event->dtDueStr()));
    }
    mText.append(i18n("<p><b>Priority:</b> %2</p>")
                 .arg(QString::number(event->priority())));

    mText.append(i18n("<p><i>%1 % completed</i></p>")
                 .arg(event->percentComplete()));
     addTag("p",i18n("<b>Access: </b>") +event->secrecyStr() );
    formatCategories(event);
    if (!event->description().isEmpty()) {
        addTag("p",i18n("<b>Details: </b>"));
        addTag("p",event->description());
    }



    formatReadOnly(event);
    formatAttendees(event);

}

void KIncidenceFormatter::setJournal(Journal*  )
{

}

void KIncidenceFormatter::formatCategories(Incidence *event)
{
 if (!event->categoriesStr().isEmpty()) {
     addTag("p",i18n("<b>Categories: </b>")+event->categoriesStr() );
     //mText.append(event->categoriesStr());
 }
}
void KIncidenceFormatter::addTag(const QString & tag,const QString & text)
{
  int number=text.contains("\n");
    QString str = "<" + tag + ">";
    QString tmpText=text;
    QString tmpStr=str;
    if(number !=-1) 
        {
            if (number > 0) {
                int pos=0;
                QString tmp;
                for(int i=0;i<=number;i++) {
                    pos=tmpText.find("\n");
                    tmp=tmpText.left(pos);
                    tmpText=tmpText.right(tmpText.length()-pos-1);
                    tmpStr+=tmp+"<br>";
                }
            }
            else tmpStr += tmpText;
            tmpStr+="</" + tag + ">";
            mText.append(tmpStr);
        }
    else
        {
            str += text + "</" + tag + ">";
            mText.append(str);
        }
}

void KIncidenceFormatter::formatAttendees(Incidence *event)
{
 Attendee::List attendees = event->attendees();
  if ( attendees.count() ) {
    KIconLoader* iconLoader = new KIconLoader();
    QString iconPath = iconLoader->iconPath( "mail_generic", KIcon::Small );
    addTag( "h3", i18n("Organizer") );
    mText.append( "<ul><li>" );
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self();
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail( event->organizer() );
    KABC::Addressee o = addressList.first();
    if ( !o.isEmpty() && addressList.size() < 2 ) {
      addLink( "uid" + o.uid(), o.formattedName() );
    } else {
      mText.append( event->organizer() );
    }
#else
    mText.append( event->organizer() );
#endif
    if ( !iconPath.isNull() ) {
      addLink( "mailto:" + event->organizer(),
               "<img src=\"" + iconPath + "\">" );
    }
    mText.append( "</li></ul>" );

    addTag( "h3", i18n("Attendees") );
    mText.append( "<ul>" );
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
#ifndef KORG_NOKABC
      if ( a->name().isEmpty() ) {
        addressList = add_book->findByEmail( a->email() );
        KABC::Addressee o = addressList.first();
        if ( !o.isEmpty() && addressList.size() < 2 ) {
          addLink( "uid" + o.uid(), o.formattedName() );
        } else {
          mText += "<li>";
          mText.append( a->email() );
          mText += "\n";
        }
      } else {
        mText += "<li><a href=\"uid:" + a->uid() + "\">";
        if ( !a->name().isEmpty() ) mText += a->name();
        else mText += a->email();
        mText += "</a>\n";
      }
#else
      mText += "<li><a href=\"uid:" + a->uid() + "\">";
      if ( !a->name().isEmpty() ) mText += a->name();
      else mText += a->email();
      mText += "</a>\n";
#endif
      kdDebug(5850) << "formatAttendees: uid = " << a->uid() << endl;

      if ( !a->email().isEmpty() ) {
        if ( !iconPath.isNull() ) {
          mText += "<a href=\"mailto:" + a->name() +" "+ "<" + a->email() + ">" + "\">";
          mText += "<img src=\"" + iconPath + "\">";
          mText += "</a>\n";
        }
      }
    }
    mText.append( "</li></ul>" );
  }
}

void KIncidenceFormatter::formatReadOnly(Incidence *event)
{
 if (event->isReadOnly()) {
        addTag("p","<em>(" + i18n("read-only") + ")</em>");
    }
}
void KIncidenceFormatter::addLink( const QString &ref, const QString &text,
                             bool newline )
{
  mText += "<a href=\"" + ref + "\">" + text + "</a>";
  if ( newline ) mText += "\n";
}
