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
#include <qcolor.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>

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
#include "EmpathJob.h"
#include "EmpathJobScheduler.h"

Empath * Empath::EMPATH = 0;

    void
Empath::start()
{
    if (!empath)
        (void)new Empath;
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
    
    using namespace EmpathConfig;

    DFLT_Q_1   = new QColor(Qt::darkBlue);
    DFLT_Q_2   = new QColor(Qt::darkCyan);
    DFLT_LINK  = new QColor(Qt::blue);
    DFLT_NEW   = new QColor(Qt::darkRed);

    // Don't do dollar expansion by default.
    // Possible security hole.
    KGlobal::config()->setDollarExpansion(false);    

    KGlobal::dirs()->addResourceType("indices", "share/apps/empath/indices");
    KGlobal::dirs()->addResourceType("cache",   "share/apps/empath/cache");

    updateOutgoingServer(); // Must initialise the pointer.
    
    composer_ = new EmpathComposer;
    jobScheduler_ = new EmpathJobScheduler;

    QObject::connect(
        composer_, SIGNAL(composeFormComplete(EmpathComposeForm)),
        SIGNAL(newComposer(EmpathComposeForm)));
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

    viewFactory_.init();
}

Empath::~Empath()
{
    using namespace EmpathConfig;

    delete DFLT_Q_1;
    delete DFLT_Q_2;
    delete DFLT_LINK;
    delete DFLT_NEW;

    DFLT_Q_1 = DFLT_Q_2 = DFLT_LINK = DFLT_NEW = 0;
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

    RMM::RMessage
Empath::message(const EmpathURL & source)
{
    empathDebug("message(" + source.asString() + ") called");

    QDictIterator<EmpathCachedMessage> it(cache_);

    for (; it.current(); ++it) {
        if (it.current()->refCount() == 0) {
            empathDebug("Removing unreferenced message " + it.currentKey());
            cache_.remove(it.currentKey());
            break; // One at a time.
        }
    }
    
    EmpathCachedMessage * cached = cache_[source.asString()];

    if (cached == 0) {
        empathDebug("Message \"" + source.asString() + "\" not in cache !");
        return RMM::RMessage();
    }

    return cached->message();
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
Empath::cacheMessage(const EmpathURL & url, RMM::RMessage m)
{
    EmpathCachedMessage * cached = cache_[url.asString()];

    if (cached == 0) {

        empathDebug("Not in cache. Adding " + url.asString());
        empathDebug(url.asString());
        cache_.insert(url.asString(), new EmpathCachedMessage(m));

    } else {

        empathDebug("Already in cache. Referencing " + url.asString());
        cached->ref();
    }
}

// Inline (later on, when we're not adjusting them) methods follow

    EmpathJobID
Empath::copy(const EmpathURL & from, const EmpathURL & to)
{ return jobScheduler_->enqueue(new EmpathCopyJob(from, to)); }

    EmpathJobID
Empath::move(const EmpathURL & from, const EmpathURL & to)
{ return jobScheduler_->enqueue(new EmpathMoveJob(from, to)); }

    EmpathJobID
Empath::retrieve(const EmpathURL & url)
{ return jobScheduler_->enqueue(new EmpathRetrieveJob(url)); }

    EmpathJobID
Empath::write(RMM::RMessage & msg, const EmpathURL & folder)
{ return jobScheduler_->enqueue(new EmpathWriteJob(msg, folder)); }

    EmpathJobID
Empath::remove(const EmpathURL & url)
{ return jobScheduler_->enqueue(new EmpathRemoveJob(url)); }

    EmpathJobID
Empath::remove(const EmpathURL & folder, const QStringList & messageIDList)
{ return jobScheduler_->enqueue(new EmpathRemoveJob(folder, messageIDList)); }

    EmpathJobID
Empath::mark(const EmpathURL & url, RMM::MessageStatus status)
{ return jobScheduler_->enqueue(new EmpathMarkJob(url, status)); }

    EmpathJobID
Empath::mark(const EmpathURL & f, const QStringList & l, RMM::MessageStatus s)
{ return jobScheduler_->enqueue(new EmpathMarkJob(f, l, s)); }

    EmpathJobID
Empath::createFolder(const EmpathURL & url)
{ return jobScheduler_->enqueue(new EmpathCreateFolderJob(url)); }

    EmpathJobID
Empath::removeFolder(const EmpathURL & url)
{ return jobScheduler_->enqueue(new EmpathRemoveFolderJob(url)); }

    void
Empath::send(RMM::RMessage & m)
{ mailSender_->send(m); }

    void
Empath::queue(RMM::RMessage & m)
{ mailSender_->queue(m); }

    void
Empath::sendQueued()
{ mailSender_->sendQueued(); }

    void
Empath::s_newMailArrived()
{ emit(newMailArrived()); }

    void
Empath::filter(const EmpathURL & m)
{ filterList_.filter(m); }

    void
Empath::s_setup(SetupType t, QWidget * parent)
{ emit(setup(t, parent)); }

    void
Empath::s_about(QWidget * parent)
{ emit(about(parent)); }

    void
Empath::s_newTask(EmpathTask * t)
{ emit(newTask(t)); }

    void 
Empath::s_compose(const QString & recipient)
{ composer_->newComposeForm(recipient); }

    void
Empath::s_reply(const EmpathURL & url)
{ composer_->newComposeForm(ComposeReply, url); }

    void
Empath::s_replyAll(const EmpathURL & url)
{ composer_->newComposeForm(ComposeReplyAll, url); }

    void
Empath::s_forward(const EmpathURL & url)
{ composer_->newComposeForm(ComposeForward, url); }

    void
Empath::s_bounce(const EmpathURL & url)
{ composer_->newComposeForm(ComposeBounce, url); }

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

    EmpathViewFactory &
Empath::viewFactory()
{ return viewFactory_; } 

// vim:ts=4:sw=4:tw=78
