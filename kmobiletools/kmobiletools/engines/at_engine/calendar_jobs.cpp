/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "calendar_jobs.h"
#include "engine.h"
#include "kmobiletoolsat_engine.h"

#include <qregexp.h>
#include <qstring.h>
#include <qdatetime.h>
#include <libkcal/alarm.h>
#include <kdeversion.h>

FetchCalendar::FetchCalendar(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, kmobiletoolsAT_engine* parent)
    : kmobiletoolsATJob(pjob, device, parent)
{
    p_calendar=engine->engineData()->calendar();
    p_calendar->clear();
}

void FetchCalendar::run()
{
    engine->suspendStatusJobs(true );
    if(engine->getATAbilities().isMotorola())
    {
        fetchMotorolaCalendar();
        return;
    }
}

void FetchCalendar::fetchMotorolaCalendar()
{
    kDebug() <<"void FetchCalendar::fetchMotorolaCalendar()";
    QString buffer;
    QRegExp regexp;
    buffer=p_device->sendATCommand(this,  "AT+MDBL=1\r" );
    if(KMobileTools::SerialManager::ATError(buffer)) return;
    buffer=p_device->sendATCommand(this, "AT+MDBR=?\r" );
    if(KMobileTools::SerialManager::ATError(buffer)) { p_device->sendATCommand(this,  "AT+MDBL=0\r" ); return; }
    buffer=formatBuffer(buffer).grep("MDBR").first();
    regexp.setPattern( "^[+]MDBR:[\\s]*([\\d]*),.*");
    regexp.search(buffer);
    int maxcal=regexp.cap(1).toInt();
    kDebug() <<"Max number of calendar entries:" << maxcal;
    QStringList entries;
    for(int i=0; i<maxcal; i+=10)
    {
        buffer=p_device->sendATCommand(this,  QString("AT+MDBR=%1,%2\r")
                .arg(i).arg( (i+10 < maxcal) ? (i+10) : (maxcal ) )
                , 200 );
        entries+= formatBuffer(buffer).grep("MDBR");
    }
    QStringList::Iterator it;
    int index; QString text; bool timed; bool enabled;
    KDateTime startDT, alDT; int duration, repeat;
    QDate tempDate; int tempyear, tempmonth, tempday;
    for(it=entries.begin(); it!=entries.end(); ++it)
    {
        regexp.setPattern("^[+]MDBR:[\\s]*([\\d]),(\"[^\"]*[^,]*|[\\dA-F]*),([\\d]*),([\\d]*)");
        regexp.search(*it);
        index=regexp.cap(1).toInt();
        text=decodeString(regexp.cap(2));
        timed=(bool) regexp.cap(3).toInt();
        enabled=(bool) regexp.cap(4).toInt();
        kDebug() <<"Index=" << index <<"|| Text=" << text <<"|| Timed=" << timed <<"|| Enabled=" << enabled <<"||end";
        buffer=(*it).replace(regexp.cap(0), "");
        regexp.setPattern(",\"([\\d:]*)\",\"([\\d-]*)\",([\\d]*),\"([\\d:]*)\",\"([\\d-]*)\",([\\d]*)");
        regexp.search(buffer);
        startDT.setTime( QTime::fromString(regexp.cap(1) ) );
        alDT.setTime( QTime::fromString(regexp.cap(4) ) );
        repeat=regexp.cap(6).toInt();
        duration=regexp.cap(3).toInt();

        buffer=regexp.cap(2);
        tempyear=buffer.section('-',2,2).toInt();
        tempyear= (tempyear < 100) ? (tempyear+2000) : tempyear;
        tempmonth=buffer.section('-',0,0).toInt();
        tempday=buffer.section('-',1,1).toInt();
        tempDate.setYMD( tempyear, tempmonth, tempday );
        startDT.setDate(tempDate);
        kDebug() <<"Setdate args for" << buffer <<":" << tempyear <<"," << tempmonth <<"," << tempday;

        buffer=regexp.cap(5);
        tempyear=buffer.section('-',2,2).toInt();
        tempyear= (tempyear < 100) ? (tempyear+2000) : tempyear;
        tempmonth=buffer.section('-',0,0).toInt();
        tempday=buffer.section('-',1,1).toInt();
        tempDate.setYMD( tempyear, tempmonth, tempday );
        alDT.setDate(tempDate);
        kDebug() <<"Setdate args for" << buffer <<":" << tempyear <<"," << tempmonth <<"," << tempday;

        kDebug() <<"Start time=" << startDT.time() <<"|| Start Date="
                << startDT.date() << "|| Duration="
                << duration << "|| Alarm time="
                << alDT.time() << "|| Alarm date="
                << alDT.date() << "|| Repeat="
                << repeat << "|| End\n";
        KCal::Event *event=new KCal::Event();
        if( startDT.isValid () &&  duration!=1440 ) event->setFloats(false); else event->setFloats(true);
        event->setDtStart(startDT);
        event->setDuration( duration*60);
        switch( repeat ){
            case 1:
                event->recurrence ()->setDaily(1);
                break;
            case 2:
                event->recurrence()->setWeekly(1);
                break;
            case 3:
                event->recurrence()->setMonthly(1);
                break;
            case 4:
                event->recurrence()->setWeekly(4);
                break;
            case 5:
                event->recurrence()->setYearly(1);
                break;
            default:
                event->recurrence()->clear();
        }
        event->setDescription(text);
        if(enabled)
        {
            KCal::Alarm *alarm=event->newAlarm();
//             if( alDT.isValid () ) alarm->setFloats(false); else alarm->setFloats(true);
            alarm->setText(text);
            alarm->setDisplayAlarm(text);
            alarm->setTime(alDT);
            alarm->setStartOffset(KCal::Duration(startDT, alDT) );
            alarm->setType(KCal::Alarm::Display);
            alarm->setEnabled(true);
//             event->addAlarm(alarm);
        }
        p_calendar->append(event);
    }
    p_calendar->dump();
    p_device->sendATCommand(this,  "AT+MDBL=0\r", 100 );
}

#include "calendar_jobs.moc"
