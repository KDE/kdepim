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
#include <klocale.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathComposer.h"
#include "EmpathMailboxList.h"
#include "EmpathMailSenderSendmail.h"
#include "EmpathMailSenderQmail.h"
#include "EmpathMailSenderSMTP.h"
#include "EmpathFilterList.h"
#include "EmpathTask.h"

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
    delete composer_;
    composer_ = 0;

    delete this;
}

Empath::Empath()
    :   QObject((QObject *)0L, "Empath"),
        mailSender_(0),
        seq_(0)
{
    EMPATH = this;
    updateOutgoingServer(); // Must initialise the pointer.
    
    composer_ = new EmpathComposer;
    QObject::connect(
        composer_, SIGNAL(composeFormComplete(const EmpathComposer::Form &)),
        SIGNAL(newComposer(const EmpathComposer::Form &)));
}

    void
Empath::init()
{
    processID_ = getpid();
    pidStr_.setNum(processID_);
    
    _saveHostName();
    _setStartTime();
 
    KConfig * c = KGlobal::config();

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_FOLDERS);

    QString s(i18n("Local"));

    inbox_  .setMailboxName(s);
    outbox_ .setMailboxName(s);
    sent_   .setMailboxName(s);
    drafts_ .setMailboxName(s);
    trash_  .setMailboxName(s);
    
    inbox_  .setFolderPath  (c->readEntry(FOLDER_INBOX,   i18n("Inbox")));
    outbox_ .setFolderPath  (c->readEntry(FOLDER_OUTBOX,  i18n("Outbox")));
    sent_   .setFolderPath  (c->readEntry(FOLDER_SENT,    i18n("Sent")));
    drafts_ .setFolderPath  (c->readEntry(FOLDER_DRAFTS,  i18n("Drafts")));
    trash_  .setFolderPath  (c->readEntry(FOLDER_TRASH,   i18n("Trash")));
   
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
        empathDebug("Message data has been deleted in cache object !");
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

    if (!m->referencedBy(xinfo)) {
        empathDebug("Hey, you don't OWN this message ! It's staying cached !");
        return;
    }

    m->deref(xinfo);

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
Empath::s_saveNameReady(const EmpathURL & url, QString name)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0)
        return;

    EmpathJobInfo j(RetrieveMessage, url, name, "save");
    m->queueJob(j);

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

    void
Empath::copy(
    const EmpathURL & from,
    const EmpathURL & to,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(from);

    if (m == 0)
        return;

    EmpathJobInfo ji(CopyMessage, from, to, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::move(
    const EmpathURL & from,
    const EmpathURL & to,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(from);

    if (m == 0)
        return;

    EmpathJobInfo ji(MoveMessage, from, to, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::retrieve(
    const EmpathURL & url,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0)
        return;

    EmpathJobInfo ji(RetrieveMessage, url, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::write(
    const EmpathURL & folder,
    RMM::RMessage & msg,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(folder);

    if (m == 0)
        return;

    EmpathJobInfo ji(folder, EmpathURL(), msg, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::remove(
    const EmpathURL & url,
    QString extraInfo = QString::null)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0)
        return;

    EmpathJobInfo ji(
        RemoveMessage, url, QStringList(url.messageID()),
        QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::remove(
    const EmpathURL & folder,
    const QStringList & list,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(folder);

    if (m == 0)
        return;

    EmpathJobInfo ji(RemoveMessage, folder, list, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void 
Empath::mark(
    const EmpathURL & url,
    RMM::MessageStatus status,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0)
        return;

    EmpathJobInfo ji(
        MarkMessage, url, QStringList(), status, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void    
Empath::mark(
    const EmpathURL & url,
    const QStringList & list,
    RMM::MessageStatus status,
    QString extraInfo)
{
    EmpathMailbox * m = mailbox(url);

    if (m == 0)
        return;

    EmpathJobInfo ji(
        MarkMessage, url, list, status, QString::null, extraInfo);
    
    m->queueJob(ji);
}

    void
Empath::createFolder(const EmpathURL & url, QString extraInfo)
{
    EmpathMailbox * m = mailbox(url);
    
    if (m == 0)
        return;

    EmpathJobInfo ji(CreateFolder, url, QString::null, extraInfo);

    m->queueJob(ji);
}

    void
Empath::jobFinished(EmpathJobInfo ji)
{
    emit(jobComplete(ji));
}

void Empath::send(RMM::RMessage & m)     { mailSender_->send(m);     }
void Empath::queue(RMM::RMessage & m)    { mailSender_->queue(m);    }
void Empath::sendQueued()                { mailSender_->sendQueued();}
void Empath::s_newMailArrived()          { emit(newMailArrived());   }
void Empath::filter(const EmpathURL & m) { filterList_.filter(m);    }

void Empath::s_setup(SetupType t, QWidget * parent)
{ emit(setup(t, parent)); }

void Empath::s_about(QWidget * parent)         { emit(about(parent));          }

void Empath::s_newTask(EmpathTask * t) { emit(newTask(t)); }

    void 
Empath::s_compose(const QString & recipient)
{ composer_->newComposeForm(recipient); }

    void
Empath::s_reply(const EmpathURL & url)
{ composer_->newComposeForm(EmpathComposer::ComposeReply, url); }

    void
Empath::s_replyAll(const EmpathURL & url)
{ composer_->newComposeForm(EmpathComposer::ComposeReplyAll, url); }

    void
Empath::s_forward(const EmpathURL & url)
{ composer_->newComposeForm(EmpathComposer::ComposeForward, url); }

    void
Empath::s_bounce(const EmpathURL & url)
{ composer_->newComposeForm(EmpathComposer::ComposeBounce, url); }

    void
Empath::s_bugReport()
{ composer_->bugReport(); }

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

    void
Empath::s_showFolder(const EmpathURL & u, unsigned int i)
{ emit(showFolder(u, i)); }

    void
Empath::s_updateFolderLists()
{ emit(updateFolderLists()); }
    
    void
Empath::s_syncFolderLists()
{ emit(syncFolderLists()); }

    EmpathURL
Empath::inbox() const
{ return inbox_; }

    EmpathURL
Empath::outbox() const
{ return outbox_; }

    EmpathURL
Empath::sent() const
{ return sent_; }
    
    EmpathURL
Empath::drafts() const
{ return drafts_; }
    
    EmpathURL
Empath::trash() const
{ return trash_; }

// vim:ts=4:sw=4:tw=78
