/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qregexp.h>
#include <qdir.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kio/job.h>

// Local includes
#include "EmpathMailboxPOP3.h"
#include "EmpathFolderList.h"
#include "Empath.h"
#include "EmpathUtilities.h"

EmpathMailboxPOP3::EmpathMailboxPOP3(const QString & name)
    :   EmpathMailbox       (name),
        serverPort_         (110),
        logging_            (false),
        numMessages_        (0),
        mailboxSize_        (0),
        logFileOpen_        (false),
        authenticationTries_(8)
{
    type_ = POP3;
    typeString_ = "POP3";
    
//    job = new KIOJob();
    
//    job->setGUImode(KIOJob::NONE);
    
    commandQueue_.setAutoDelete(true);
}

EmpathMailboxPOP3::~EmpathMailboxPOP3()
{
//    delete job;
}

    void
EmpathMailboxPOP3::init()
{
    loadConfig();
    
//    QObject::connect(
//        job,    SIGNAL(sigFinished(int)),
//        this,   SLOT(s_jobFinished(int)));
    
//    QObject::connect(
//        job,    SIGNAL(sigCanceled(int)),
//        this,   SLOT(s_jobCancelled(int)));
  
//    QObject::connect(
//        job,    SIGNAL(sigError(int, int, const char *)),
//        this,   SLOT(s_jobError(int, int, const char *)));
    
//    QObject::connect(
//        job,    SIGNAL(sigData(int, const char *, int)),
//        this,   SLOT(s_jobData(int, const char *, int)));
    
    EmpathURL url(url_);
    url.setFolderPath(i18n("Inbox"));
    
    EmpathFolder * folder_inbox = new EmpathFolder(url);
    folderList_.insert(url.folderPath(), folder_inbox);
}

    bool
EmpathMailboxPOP3::alreadyHave()
{
    // STUB
    return false;
}

    void
EmpathMailboxPOP3::_enqueue(EmpathPOPCommand::Type t, int i)
{
    commandQueue_.enqueue(new EmpathPOPCommand(t, i));
    
    if (commandQueue_.count() == 1)
        _nextCommand();
}

    void
EmpathMailboxPOP3::_nextCommand()
{
    if (commandQueue_.isEmpty())
        return;

    // TODO: KIO will ask user for password if not given.
    // We really want to save this, so we should ask ourselves.

    QString prefix =
        "pop://" + username_ + ":" + password_ + "@" + serverAddress_ + "/";
    
    QString command = prefix + commandQueue_.head()->command();
    
    empathDebug("Doing `" + command + "'");
//    job->get(command.ascii());
}

    void
EmpathMailboxPOP3::s_checkMail()
{
    _enqueue(EmpathPOPCommand::Index, -1);
}

    bool
EmpathMailboxPOP3::newMail() const
{
    // STUB
    return false;
}

    void
EmpathMailboxPOP3::sync(const EmpathURL &)
{
    // TODO
//    index_.clear();
//    _enqueue(EmpathPOPCommand::UIDL);
}

   EmpathURL
EmpathMailboxPOP3::path()
{
    return url_;
}

    
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////// KIOJOB SLOTS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    void
EmpathMailboxPOP3::s_jobCancelled(int id) 
{
    empathDebug("s_jobCancelled(" + QString().setNum(id) + ") called");
    commandQueue_.dequeue();
    _nextCommand();
}

    void
EmpathMailboxPOP3::s_jobFinished(int id) 
{
    empathDebug(QString().setNum(id));

    if (commandQueue_.isEmpty()) {
        // Whoah ! We should be waiting for completion.
        empathDebug("Command queue empty !");
        return;
    }
    
    switch (commandQueue_.head()->type()) {
            
        case EmpathPOPCommand::Stat:
            // STUB
//            commandQueue_.head()->jobInfo().done(false);
            break;
        
        case EmpathPOPCommand::Index:
            {
                EmpathPOPIndexIterator it(index_);
               
                EmpathURL messageURL(url_);

                for (; it.current(); ++it) {
                    messageURL.setMessageID(it.current()->id());
                    empath->filter(messageURL);
                }

//                commandQueue_.head()->jobInfo().done(true);
            }
            break;
        
        case EmpathPOPCommand::Get:
            {
                QCString data = commandQueue_.head()->data();
                EmpathURL from;// = commandQueue_.head()->jobInfo().from();

                empath->cacheMessage(from, RMM::Message(data));

//                commandQueue_.head()->jobInfo().done(true);
            }
            
            break;
        
        case EmpathPOPCommand::Remove:
//            commandQueue_.head()->jobInfo().done(true);
            break;
        
        default:
            break;
    }

    commandQueue_.dequeue();
    _nextCommand();
}

    void
EmpathMailboxPOP3::s_jobError(int id, int errorID, const char * text) 
{
    empathDebug("s_jobError(" + QString().setNum(id) + ") called");
    empathDebug("Error ID == " + QString().setNum(errorID));
    empathDebug("Error text == " + QString(text));
    commandQueue_.dequeue();
}

    void
EmpathMailboxPOP3::s_jobData(int, const char * data, int)
{
    empathDebug("data == `" + QString(data) + "'");
    
    QCString s(data);
    if (s.isEmpty()) {
        empathDebug("Data is empty !");
        return;
    }

    if (commandQueue_.isEmpty()) {
        // Whoah ! We should be waiting for completion.
        empathDebug("Command queue empty !");
        return;
    }
    
    switch (commandQueue_.head()->type()) {
            
        case EmpathPOPCommand::Stat:
            {
                int i = s.find(' ');
            
                msgsInSpool_ = s.left(i).toInt();
            
                octetsInSpool_ = s.mid(i + 1).toInt();
            }
            break;
        
        case EmpathPOPCommand::Index:
            {
                int i = s.find(' ');
            
                EmpathPOPIndexEntry * e =
                    new EmpathPOPIndexEntry(
                        QString(s).left(i).toInt(),
                        url_.mailboxName() + ":" + QString(s).mid(i + 1));
                index_.append(e);
            }
            break;
        
        case EmpathPOPCommand::Get:
            
            commandQueue_.head()->data().append(s);
            
            break;
        
        case EmpathPOPCommand::Remove:
            break;
        
        default:
            break;
    }
}

    RMM::Message
EmpathMailboxPOP3::retrieveMessage(const EmpathURL & /* url */)
{
    // STUB
    RMM::Message retval;
    return retval;
}

    QString
EmpathMailboxPOP3::writeMessage(
    RMM::Message &,
    const EmpathURL &
)
{
    // We don't do this.
    return QString::null;
}

    bool
EmpathMailboxPOP3::removeMessage(const EmpathURL & /* url */)
{
    // STUB
    return false;
}
        
    EmpathSuccessMap
EmpathMailboxPOP3::removeMessage(
    const EmpathURL & /* folder */,
    const QStringList & /* messageIDList */
)
{
    // STUB
    EmpathSuccessMap retval;
    return retval;
}

    bool
EmpathMailboxPOP3::markMessage(const EmpathURL &, EmpathIndexRecord::Status)
{
    // We don't do this.
    return false;
}

    EmpathSuccessMap
EmpathMailboxPOP3::markMessage(
    const EmpathURL &,
    const QStringList &,
    EmpathIndexRecord::Status)
{
    // We don't do this.
    EmpathSuccessMap retval;
    return retval;
}

    bool
EmpathMailboxPOP3::createFolder(const EmpathURL &)
{
    // We don't do this.
    return false;
}

    bool
EmpathMailboxPOP3::removeFolder(const EmpathURL &)
{
    // We don't do this.
    return false;
}
 
//////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// CONFIG ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    void
EmpathMailboxPOP3::saveConfig()
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("Mailbox_" + url_.mailboxName());

    c->writeEntry("Type",           (unsigned long)type_);
    c->writeEntry("Address",        serverAddress_);
    c->writeEntry("Port",           serverPort_);
    c->writeEntry("Username",       username_);
    c->writeEntry("Password",       password_);
    c->writeEntry("LoggingPolicy",  logging_);
    c->writeEntry("LogFilePath",    logFilePath_);
    c->writeEntry("LogDisposal",    logFileDisposalPolicy_);
    c->writeEntry("MaxLogSize",     maxLogFileSize_);
    c->writeEntry("Check",          autoCheck_);
    c->writeEntry("CheckInterval",  autoCheckInterval_);
}

    void
EmpathMailboxPOP3::loadConfig()
{
    KConfig * c = KGlobal::config();
    
    c->setGroup("Mailbox_" + url_.mailboxName());

    serverAddress_          = c->readEntry              ("Address");
    serverPort_             = c->readUnsignedNumEntry   ("Port", 110);
    
    c->setDollarExpansion(true);
    username_               = c->readEntry              ("Username", "$USER");
    c->setDollarExpansion(false);
    
    password_               = c->readEntry              ("Password", "");
    logging_                = c->readBoolEntry          ("LoggingPolicy",false);
    logFilePath_            = c->readEntry              ("LogFilePath", "");
    logFileDisposalPolicy_  = c->readBoolEntry          ("LogDisposal", false);
    maxLogFileSize_         = c->readUnsignedNumEntry   ("LogMaxSize",    10);
    autoCheck_              = c->readBoolEntry          ("Check", true);
    autoCheckInterval_      = c->readUnsignedNumEntry   ("CheckInterval", 5);
}

// Set methods
        
    void
EmpathMailboxPOP3::setServerAddress(const QString & serverAddress)
{
    serverAddress_    = serverAddress;
}

    void
EmpathMailboxPOP3::setServerPort(Q_UINT32 serverPort)
{
    serverPort_ = serverPort;
}

    void
EmpathMailboxPOP3::setUsername(const QString & username)
{
    username_ = username;
}

    void
EmpathMailboxPOP3::setUseAPOP(bool yn)
{
    useAPOP_ = yn;
}

    void
EmpathMailboxPOP3::setPassword(const QString & password)
{
    password_ = password;
}

    void
EmpathMailboxPOP3::setLoggingPolicy(bool policy)
{
    loggingPolicy_ = policy;
}

    void
EmpathMailboxPOP3::setLogFilePath(const QString & logPath)
{
    logFilePath_ = logPath;
}

    void
EmpathMailboxPOP3::setLogFileDisposalPolicy(bool policy)
{
    logFileDisposalPolicy_ = policy;
}

    void
EmpathMailboxPOP3::setMaxLogFileSize(Q_UINT32 maxSize)
{
    maxLogFileSize_ = maxSize; 
}

    void
EmpathMailboxPOP3::setRetrieveIfHave(bool yn)
{
    retrieveIfHave_ = yn;
}

// Get methods
        
    QString
EmpathMailboxPOP3::serverAddress()
{
    return serverAddress_;
}

    Q_UINT32
EmpathMailboxPOP3::serverPort()
{
    return serverPort_;
}

    QString
EmpathMailboxPOP3::username()
{
    return username_;
}

    QString
EmpathMailboxPOP3::password()
{
    return password_;
}

    bool
EmpathMailboxPOP3::useAPOP()
{
    return useAPOP_;
}

    bool
EmpathMailboxPOP3::loggingPolicy()
{ 
    return loggingPolicy_;
}

    QString
EmpathMailboxPOP3::logFilePath()
{
    return logFilePath_;
}

    bool
EmpathMailboxPOP3::logFileDisposalPolicy()
{
    return logFileDisposalPolicy_;
}

    Q_UINT32
EmpathMailboxPOP3::maxLogFileSize()
{
    return maxLogFileSize_;
}

    bool
EmpathMailboxPOP3::retrieveIfHave()
{
    return retrieveIfHave_;
}

    bool
EmpathMailboxPOP3::logging()
{
    return logging_;
}

    void
EmpathMailboxPOP3::setLogging(bool policy)
{
    logging_ = policy;
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// COMMANDS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

EmpathPOPCommand::EmpathPOPCommand(EmpathPOPCommand::Type t, int n)
    :   type_(t),
        msgNo_(n)
{
    switch (t) {
        
        case Stat:      command_ = "stat";      break;
        case Index:     command_ = "index";     break;
        case Get:       command_ = "download";  break;
        case Remove:    command_ = "remove";    break;
        default:                                break;
    }
    
    command_ += '/';

    if (n != -1)
        command_ += QString().setNum(n);
}

EmpathPOPCommand::~EmpathPOPCommand()
{
    // Empty.
}

    QString
EmpathPOPCommand::command()
{
    return command_;
}

    EmpathPOPCommand::Type
EmpathPOPCommand::type()
{
    return type_;
}

    int
EmpathPOPCommand::messageNumber()
{
    return msgNo_;
}

    QCString &
EmpathPOPCommand::data()
{
    return data_;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////// INDEX //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

EmpathPOPIndexEntry::EmpathPOPIndexEntry(int i, const QString & s)
    :    number_(i),
        id_(s)
{
}

EmpathPOPIndexEntry::~EmpathPOPIndexEntry()
{
    // Empty.
}

    int
EmpathPOPIndexEntry::number()
{
    return number_;
}

    QString
EmpathPOPIndexEntry::id()
{
    return id_;
}

EmpathPOPIndex::EmpathPOPIndex()
{
    // Empty.
}

EmpathPOPIndex::~EmpathPOPIndex()
{
    // Empty.
}

    int
EmpathPOPIndex::compareItems(EmpathPOPIndexEntry * i1, EmpathPOPIndexEntry * i2)
{
    return i1->number() - i2->number();
}
// vim:ts=4:sw=4:tw=78
