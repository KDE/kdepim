/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma implementation "Empath.h"
#endif

// System includes
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

// Qt includes
#include <qtimer.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathMailboxList.h"
#include "EmpathIndexRecord.h"
#include "EmpathMailSenderSendmail.h"
#include "EmpathMailSenderQmail.h"
#include "EmpathMailSenderSMTP.h"
#include "EmpathFilterList.h"
#include "EmpathTask.h"
#include "EmpathTaskTimer.h"

Empath * Empath::EMPATH = 0;
bool Empath::started_ = false;

    void
Empath::start()
{
    if (!started_) {
        started_ = true;
        (void)new Empath;
    }
}
        
    void
Empath::shutdown()
{
    delete mailSender_;
    mailSender_ = 0;

    delete this;
}

Empath::Empath()
    :   QObject(),
        mailSender_(0),
        seq_(0)
{
    EMPATH = this;
    updateOutgoingServer(); // Must initialise the pointer.
}

    void
Empath::init()
{
    processID_ = getpid();
    pidStr_.setNum(processID_);
    
    _saveHostName();
    _setStartTime();
    
    mailboxList_.loadConfig();
    filterList_.loadConfig();
}

Empath::~Empath()
{
}

    void
Empath::s_saveConfig()
{
    filterList_.saveConfig();
    mailboxList_.saveConfig();
    KGlobal::config()->sync();
}

    void
Empath::updateOutgoingServer()
{
    delete mailSender_;
    mailSender_ = 0;
    
    KConfig * c = KGlobal::config();
    c->setGroup(EmpathConfig::GROUP_GENERAL);
    
    EmpathMailSender::OutgoingServerType st =
        (EmpathMailSender::OutgoingServerType)
        (c->readUnsignedNumEntry(EmpathConfig::S_TYPE));
    
    switch (st) {
        
        case EmpathMailSender::Qmail:
            mailSender_ = new EmpathMailSenderQmail;
            break;
        
        case EmpathMailSender::SMTP:
            mailSender_ = new EmpathMailSenderSMTP;
            break;
            
        case EmpathMailSender::Sendmail:
        default:
            mailSender_ = new EmpathMailSenderSendmail;
            break;
    }

    CHECK_PTR(mailSender_);

    mailSender_->loadConfig();
}

    RMM::RMessage *
Empath::message(const EmpathURL & source, const QString & xinfo)
{
    empathDebug("message(" + source.asString() + ") called");
    
    // Try and get the message from the cache.

    EmpathCachedMessage * cached = cache_[source.asString()];

    if (cached == 0) {
        empathDebug("Message \"" + source.asString() + "\" not in cache !");
        return 0;
    }

    if (cached->message(xinfo) == 0) {
        empathDebug("Message \"" + source.asString() + "\" not in cache !");
        return 0;
    }
    
    return cached->message(xinfo);
}

    void
Empath::finishedWithMessage(const EmpathURL & url, const QString & xinfo)
{
    empathDebug(url.asString()); 
    EmpathCachedMessage * m = cache_[url.asString()];

    if (m == 0) {
        empathDebug("It wasn't in the cache anyways");
        return;
    }

    if (m->refCount() == 0) {
        empathDebug("Refcount has dropped to 0. Deleting.");
        cache_.remove(url.asString());
    }
}

    EmpathMailbox *
Empath::mailbox(const EmpathURL & url)
{
    return mailboxList_[url.mailboxName()];
}

    EmpathFolder *
Empath::folder(const EmpathURL & url)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0)
        return 0;

    return m->folder(url);
}

    void
Empath::copy(const EmpathURL & from, const EmpathURL & to, QString xinfo)
{
    EmpathMailbox * m_from = mailbox(from);
    
    if (m_from == 0) {
        emit(copyComplete(false, from, to, xinfo));
        return;
    }

    m_from->retrieve(from, to.asString(), xinfo);
}

    void
Empath::move(const EmpathURL & from, const EmpathURL & to, QString xinfo)
{
    EmpathMailbox * m_from = mailbox(from);
    
    if (m_from == 0) {
        emit(moveComplete(false, from, to, xinfo));
        return;
    }

    m_from->retrieve(from, to.asString(), xinfo);
}

    void
Empath::retrieve(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        emit(retrieveComplete(false, url, xinfo));
        return;
    }

    m->retrieve(url, QString::null, xinfo);
}

    EmpathURL
Empath::write(const EmpathURL & url, RMM::RMessage & msg, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        emit(writeComplete(false, url, xinfo));
        return EmpathURL();
    }
    return m->write(url, msg, QString::null, xinfo);
}

    void
Empath::createFolder(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0) {
        emit(createFolderComplete(false, url, xinfo));
        return;
    }

    m->createFolder(url, QString::null, xinfo);
}

    void
Empath::remove(const EmpathURL & url, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        emit(removeComplete(false, url, xinfo));
        return;
    }

    m->remove(url, QString::null, xinfo);
}

    void
Empath::remove(const EmpathURL & url, const QStringList & l, QString xinfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0) {
        emit(removeComplete(false, url, xinfo));
        return;
    }

    return m->remove(url, l, QString::null, xinfo);
}

    void
Empath::mark(const EmpathURL & url, RMM::MessageStatus s, QString xinfo)
{
    empathDebug("mark() called");
    
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        emit(markComplete(false, url, xinfo));
        return;
    }

    m->mark(url, s, QString::null, xinfo);
}

    void
Empath::mark(
    const EmpathURL & url,
    const QStringList & l,
    RMM::MessageStatus s,
    QString xinfo)
{
    empathDebug("mark() called");

    EmpathMailbox * m = mailbox(url);
    
    if (m == 0) {
        emit(markComplete(false, url, xinfo));
        return;
    }

    m->mark(url, l, s, QString::null, xinfo);
}

    EmpathTask *
Empath::addTask(const QString & name)
{
    EmpathTask * t = new EmpathTask(name);
    CHECK_PTR(t);
    new EmpathTaskTimer(t);
    return t;
}


// Private methods follow

    void
Empath::_setStartTime()
{
    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    startupSeconds_ = timeVal.tv_sec;
    startupSecondsStr_.setNum(startupSeconds_);
}
    void
Empath::_saveHostName()
{
    struct utsname utsName;
    if (uname(&utsName) == 0)
        hostName_ = utsName.nodename;
}

    QString
Empath::generateUnique()
{
    QString unique;

    unique =
        startupSecondsStr_ +
        '.' +
        pidStr_ +
        '_' +
        QString().setNum(seq_) +
        '.' +
        hostName_;

    ++seq_;

    return unique;
}

    void
Empath::cacheMessage
    (const EmpathURL & url, RMM::RMessage * m, const QString & xinfo)
{
    EmpathCachedMessage * cached = cache_[url.asString()];

    if (cached == 0) {

        empathDebug("Not in cache. Adding");
        cache_.insert(url.asString(), new EmpathCachedMessage(m, xinfo));

    } else {

        empathDebug("Already in cache. Referencing");
        cached->ref(xinfo);
    }
}

    void
Empath::s_retrieveComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    empathDebug("emitting retrieveComplete");
    emit(retrieveComplete(status, from, to, xinfo));
}

    void
Empath::s_retrieveComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    empathDebug("emitting retrieveComplete");
    emit(retrieveComplete(status, url, xinfo));
}

    void
Empath::s_moveComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(moveComplete(status, from, to, xinfo));
}

    void
Empath::s_copyComplete(
    bool status,
    const EmpathURL & from,
    const EmpathURL & to,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(copyComplete(status, from, to, xinfo));
}

    void
Empath::s_removeComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(removeComplete(status, url, xinfo));
}

    void
Empath::s_markComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(markComplete(status, url, xinfo));
}

    void
Empath::s_writeComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{    
    emit(writeComplete(status, url, xinfo));
}

    void
Empath::s_createFolderComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo) 
{
    emit(createFolderComplete(status, url, xinfo));
}

    void
Empath::s_removeFolderComplete(
    bool status,
    const EmpathURL & url,
    QString /* ixinfo */,
    QString xinfo)
{
    emit(removeFolderComplete(status, url, xinfo));
}

    void
Empath::s_saveNameReady(const EmpathURL & url, QString name)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0)
        return;

    m->retrieve(url, name, "save");

}

    void
Empath::s_messageReadyForSave(
    bool status, const EmpathURL & url, QString name, QString xinfo)
{
    if (xinfo != "save")
        return;
    
    if (!status) {
        empathDebug("Couldn't retrieve message for saving");
        return;
    }
    
    QFile f(name);

    if (!f.open(IO_WriteOnly)) {

        empathDebug("Couldn't write to file \"" + name + "\"");
//        QMessageBox::information(this, "Empath",
//            i18n("Sorry I can't write to that file. "
//                "Please try another filename."), i18n("OK"));

        return;
    }
    
    RMM::RMessage * m = message(url, xinfo);
    
    if (m == 0) {
        empathDebug("Couldn't get message that supposedly retrieved");
        return;
    }
    
    QString s(m->asString());

    finishedWithMessage(url, xinfo);
  
    unsigned int blockSize = 1024; // 1k blocks
    
    unsigned int fileLength = s.length();

    for (unsigned int i = 0 ; i < s.length() ; i += blockSize) {
        
        QCString outStr;
        
        if ((fileLength - i) < blockSize)
            outStr = QCString(s.right(fileLength - i));
        else
            outStr = QCString(s.mid(i, blockSize));
        
        if (f.writeBlock(outStr, outStr.length()) != (int)outStr.length()) {
            // Warn user file not written.
            
            empathDebug("Couldn't save message !");
//            QMessageBox::information(this, "Empath",
//                i18n("Sorry I couldn't write the file successfully. "
//                    "Please try another file."), i18n("OK"));
            return;
        }
    }

    f.close();
    
    QString okMessage("Message saved to %1 OK");
    s_infoMessage(okMessage.arg(name));
}


void Empath::send(RMM::RMessage & m)     { mailSender_->send(m);     }
void Empath::queue(RMM::RMessage & m)    { mailSender_->queue(m);    }
void Empath::sendQueued()                { mailSender_->sendQueued();}
void Empath::s_newMailArrived()          { emit(newMailArrived());   }
void Empath::s_newTask(EmpathTask * t)   { emit(newTask(t));         }
void Empath::filter(const EmpathURL & m) { filterList_.filter(m);    }
void Empath::s_bugReport()               { emit(bugReport());        }

void Empath::s_setupDisplay(QWidget * parent)  { emit(setupDisplay(parent));   }
void Empath::s_setupIdentity(QWidget * parent) { emit(setupIdentity(parent));  }
void Empath::s_setupSending(QWidget * parent)  { emit(setupSending(parent));   }
void Empath::s_setupComposing(QWidget * parent){ emit(setupComposing(parent)); }
void Empath::s_setupAccounts(QWidget * parent) { emit(setupAccounts(parent));  }
void Empath::s_setupFilters(QWidget * parent)  { emit(setupFilters(parent));   }
void Empath::s_about(QWidget * parent)         { emit(about(parent));          }

    void 
Empath::s_compose(const QString & recipient)
{ emit(newComposer(recipient)); }

    void
Empath::s_reply(const EmpathURL & url)
{ emit(newComposer(ComposeReply, url)); }

    void
Empath::s_replyAll(const EmpathURL & url)
{ emit(newComposer(ComposeReplyAll, url)); }

    void
Empath::s_forward(const EmpathURL & url)
{ emit(newComposer(ComposeForward, url)); }

    void
Empath::s_bounce(const EmpathURL & url)
{ emit(newComposer(ComposeBounce, url)); }

    void
Empath::saveMessage(const EmpathURL & url, QWidget * parent)
{ emit(getSaveName(url, parent)); }

    void
Empath::s_configureMailbox(const EmpathURL & u, QWidget * w)
{ emit(configureMailbox(u, w)); }


    void
Empath::s_infoMessage(const QString & s)
{ emit(infoMessage(s)); }

    void
Empath::s_checkMail()
{ emit(checkMail()); }

// vim:ts=4:sw=4:tw=78
