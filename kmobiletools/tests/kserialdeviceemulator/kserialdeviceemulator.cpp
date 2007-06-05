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
#include "kserialdeviceemulator.h"

#include "kserialdeviceemulatorwidget.h"

#include <kmainwindow.h>
#include <klocale.h>
#include <kmobiletools/qserial.h>
#include <kdebug.h>
#include <unistd.h>
#include <qmutex.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <iostream>
#include <qregexp.h>

#include "commandslist.h"

#include <threadweaver/Thread.h>

static QString smsSlot, pbSlot;
static QMutex mutex;


using namespace ThreadWeaver;
KSerialDeviceEmulator::KSerialDeviceEmulator()
    : KMainWindow()
{
    setObjectName(QLatin1String( "KSerialDeviceEmulator" ));
    weaver=Weaver::instance();
//     mutex=new QMutex;
    (void) new CommandsList;
    m_widget=new KSerialDeviceEmulatorWidget( this );
    setCentralWidget( m_widget );
    serial=new KMobileTools::QSerial("/dev/ttygserial");
//     serial=new KMobileTools::QSerial("/dev/kmobiletoolsfifo.1");
    connect(serial, SIGNAL(gotData()), this, SLOT(gotData()));
    connect(weaver, SIGNAL(jobDone(Job*)), this, SLOT(jobDone( Job* )));
    connect(m_widget, SIGNAL(loadFile(const QString &)), this, SLOT(loadFile( const QString& )));
    connect(m_widget, SIGNAL(sendEvent(const QString &)), this, SLOT(sendEvent( const QString& )));
    serial->setStopBits( KMobileTools::QSerial::STOP_BITS_1 );
    serial->setDatabits( KMobileTools::QSerial::DATABITS_8 );
    serial->setFlowControl( KMobileTools::QSerial::FLOW_CONTROL_HARD );
    serial->setParity( KMobileTools::QSerial::PARITY_NONE );
    serial->setBaud(KMobileTools::QSerial::BAUD_115200);
    serial->open(QIODevice::ReadWrite);
//     serial->open(QIODevice::WriteOnly, false);
    QString buffer="ERROR12135";
    kDebug() << "FindError:" << buffer.mid( buffer.findRev( "ERROR" ) + 5) << endl;
}

using namespace KMobileTools;

KSerialDeviceEmulator::~KSerialDeviceEmulator()
{
    serial->close();
//     delete mutex;
//     serial->close();
}

#include "kserialdeviceemulator.moc"


/*!
    \fn KSerialDeviceEmulator::gotData()
 */
void KSerialDeviceEmulator::gotData()
{
//     kDebug() << "gotData()" << endl;
//     QMutexLocker locker(mutex);
    weaver->enqueue(new CommandJob(serial, weaver, "commandjob"));
}

CommandJob::CommandJob(KMobileTools::QSerial *serial, QObject* parent, const char* name )
    : ThreadWeaver::Job("serialdevice", parent, name)
{
    this->serial=serial;
}

void SendEventJob::run()
{
    QMutexLocker ml(&mutex);
    event=event.prepend( "\n" ).append("\n").replace("\n", "\r\n");
    serial->writeBlock(event.latin1(), event.length());
    serial->flush();
    thread()->msleep(50);
}

void CommandJob::run()
{
    QMutexLocker ml(&mutex);
    char buffer;
    while(!s_buffer.contains( '\r' ))
    {
//         memset(buffer, 0, sizeof(buffer));
//         serial->readC(buffer, sizeof(buffer)-1);
//         buffer=0;
        buffer=serial->getch();
        if(buffer!=-1) s_buffer+=buffer;
        thread()->msleep(1);
    }
    QString line=s_buffer.section( '\r', 0, 0 );
    line=line.trimmed();
    s_buffer=s_buffer.mid( s_buffer.find('\r')+1 );
//     thread()->msleep(10);
    gotCMD(line);
}

/*!
    \fn KSerialDeviceEmulator::gotCMD(const QString &cmd)
 */
void CommandJob::gotCMD(const QString &cmd)
{
    this->cmd.setCmd(cmd.upper() );
    QString reply=getAnswer(cmd.upper() );
    if(reply=="\nNOTFOUND\n")
    {
///         m_widget->addToLog( "ERROR CMD NOTFOUND", "darkblue");
        reply="\nERROR\n";
    }
    reply=reply.replace( "\n", "\r\n" )/*.prepend("\r\n").append("\r\n")*/;
    this->cmd.setAnswer(reply);
    thread()->msleep(70);
    serial->writeBlock(reply.latin1(), reply.length() );
    serial->flush();
    thread()->msleep(70);
//     usleep(100);
//     kDebug() << "cmd sent\n";
//     QString reply;
//     kDebug() << "Got Command: " << cmd << endl;
//     if(cmd.contains("AT+CGSN"))
//     {
//         reply="\r\n+CGSN: PROVA\r\n";
//         kDebug() << "Replying to " << cmd << " with " << reply.replace("\r", "" ).replace("\n","") << endl;
//     }
//     serial->writeBlock(reply.latin1(), reply.length() );
//     reply="\r\nOK\r\n";
//     serial->writeBlock(reply.latin1(), reply.length() );
//     usleep(100);
}



/*!
    \fn KSerialDeviceEmulator::getCommand(const QString &cmd)
 */
QString CommandJob::getAnswer(const QString &cmd)
{
//     kDebug() << "gotCMD " << cmd << endl;
//     QMutexLocker locker(mutex);
    QString reply;
    if(cmd.length() == 1 ) return QString("\nERROR\n");
    if(cmd=="AT") return QString("\nOK\n");
    QRegExp regexpPB( "AT\\+[CM]PBR=", false ),
        regexpSMS( "AT\\+[CM]MG[RL]=", false ),
        regexpSlot( "(AT\\+CP[MB]S)\\?", false );
    Command command;
    if( regexpPB.search(cmd)!=-1 || regexpSMS.search(cmd)!=-1 || regexpSlot.search(cmd)!=-1 )
    {
        bool isSMS=(regexpSMS.search(cmd)!=-1 || (regexpSlot.search(cmd)!=-1 && regexpSlot.cap(1) == "AT+CPMS") );
        if(isSMS && !CommandsList::instance()->hasSMSSlots()) {
            command=CommandsList::instance()->searchCmd(cmd);
            kDebug() << "NO SMS Slots to be found\n";
        }
        else {
//         kDebug() << "Searching for sms:" << isSMS << endl;
            regexpSlot.setPattern( "(AT\\+CP[MB]S)=\"*[\\w]+\"*");
            bool rightSlot=false;
            for(Q3ValueList<Command>::ConstIterator it=CommandsList::instance()->begin(); it!=CommandsList::instance()->end(); ++it)
            {
                if(regexpSlot.search( (*it).cmd() )!=-1)
                {
                    // If we're searching for a SMS slot and we've found a PB one, or vice versa, just continue the loop.
                    if( (regexpSlot.cap(1)=="AT+CPMS" && !isSMS ) || (regexpSlot.cap(1)=="AT+CPBS" && isSMS ) ) continue;
                    if( smsSlot==(*it).cmd() || pbSlot==(*it).cmd() )
                    {
                        kDebug() << "Searching for slot ok; was searching for " << (*it).origPos() << endl;
                        rightSlot=true;
                    }
                    else
                    {
                        kDebug() << "Was searching for a slot, but we've found another one: index: " << (*it).origPos() << endl;
                        rightSlot=false;
                    }
                    continue;
                }
                if(rightSlot && (*it).cmd()==cmd)
                {
                    command=(*it);
                    kDebug() << "Found correct slot command: " << command.origPos() << endl;
                    break;
                }
            }
        }
    } else command=CommandsList::instance()->searchCmd(cmd);
    if(command.isNull())
    {
        reply="\nNOTFOUND\n";
//         return;
    } else reply=command.answer();
    regexpSlot.setPattern( "(AT\\+CP[MB]S)=\"*[\\w]+\"*");
    if( regexpSlot.search( command.cmd()) != -1 )
    {
        if(regexpSlot.cap( 1 ) == "AT+CPBS" ) { kDebug() << "Setting pb slot: " << cmd << endl;
            pbSlot=command.cmd(); }
            else { kDebug() << "Setting sms slot: " << cmd << endl;
                smsSlot=command.cmd(); }
    }
    return reply;
}


/*!
    \fn KSerialDeviceEmulator::jobDone(ThreadWeaver::Job *)
 */
void KSerialDeviceEmulator::jobDone(ThreadWeaver::Job *job)
{
    if(!job) return;
    if(QString(job->name())=="loadfilejob")
    {
        m_widget->updateCommandListView();
        return;
    }
    if(QString(job->name())=="sendeventjob") return;

    CommandJob* cjob=static_cast<CommandJob*>(job);
    m_widget->addToLog(cjob->command().cmd(), "red");
    m_widget->addToLog(cjob->command().answer(), "darkblue");
//     delete cjob;
///     m_widget->addToLog( reply.replace("\r", "<br>"), "blue");

}


/*!
    \fn KSerialDeviceEmulator::loadFile(const QString &file)
 */
void KSerialDeviceEmulator::loadFile(const QString &file)
{
    weaver->enqueue(new LoadFileJob(file, weaver, "loadfilejob"));
}

void KSerialDeviceEmulator::sendEvent(const QString &event)
{
    kDebug() << "Send event:::" << event << ":::\n";
    weaver->enqueue(new SendEventJob(serial, event, weaver, "sendeventjob"));
}
