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
# pragma implementation "EmpathSetupWizard.h"
#endif

// System includes
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

// Qt includes
#include <qlayout.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kapp.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMailboxPOP3.h"
#include "EmpathMailboxMaildir.h"
#include "EmpathMailboxList.h"
#include "EmpathConfig.h"
#include "EmpathSetupWizard.h"

EmpathSetupWizard::EmpathSetupWizard()
    :   KWizard(0, "SetupWizard", true)
{
    setCaption(i18n("Setup Wizard"));

    welcomePage_    = new EmpathWelcomePage     (this);
    userPage_       = new EmpathUserInfoPage    (this);
    folderPage_     = new EmpathFolderInfoPage  (this);
//  imapPage_       = new EmpathIMAPInfoPage    (this);
    popPage_        = new EmpathPOPInfoPage     (this);
    reviewPage_     = new EmpathReviewPage      (this);

    addPage(welcomePage_, i18n("Welcome to Empath") );
    addPage(userPage_,    i18n("User information")  );
    addPage(folderPage_,  i18n("Folder names")      );
//  addPage(imapPage_,    i18n("IMAP mailbox")      );
    addPage(popPage_,     i18n("POP mailboxes")     );
    addPage(reviewPage_,  i18n("Review settings")   );
    
    setFinishEnabled(reviewPage_, true);
    
    QObject::connect(userPage_,
        SIGNAL(continueOK(QWidget *, bool)),
        SLOT(s_continueOK(QWidget *, bool)));
    
    QObject::connect(folderPage_,
        SIGNAL(continueOK(QWidget *, bool)),
        SLOT(s_continueOK(QWidget *, bool)));

    QObject::connect(popPage_,
        SIGNAL(continueOK(QWidget *, bool)),
        SLOT(s_continueOK(QWidget *, bool)));
}

EmpathSetupWizard::~EmpathSetupWizard()
{
    // Empty.
}

    bool
EmpathSetupWizard::appropriate(QWidget *) const
{
    return true;
}

    void
EmpathSetupWizard::accept()
{
    userPage_   ->saveConfig();
    folderPage_ ->saveConfig();
//  imapPage_   ->saveConfig();
    popPage_    ->saveConfig();

    KGlobal::config()->sync();

    KWizard::accept();
}
 
    void
EmpathSetupWizard::s_continueOK(QWidget * w, bool b)
{
    setNextEnabled(w, b);
}

EmpathWelcomePage::EmpathWelcomePage(QWidget * parent)
    :   QWidget(parent, "WelcomePage")
{
    QGridLayout * layout = new QGridLayout(this, 1, 1, 10, 10); 
    QLabel * l_welcome = new QLabel(i18n("Welcome to Empath"), this);
    layout->addWidget(l_welcome, 0, 0);
    layout->activate();
}

EmpathWelcomePage::~EmpathWelcomePage()
{
    // Empty
}

EmpathUserInfoPage::EmpathUserInfoPage(QWidget * parent)
    :   QWidget(parent, "UserInfoPage")
{
    QGridLayout * layout = new QGridLayout(this, 4, 2, 10, 10); 
    
    QLabel * l_name     = new QLabel(i18n("&Full Name"),        this);
    QLabel * l_org      = new QLabel(i18n("&Organization"),     this);
    QLabel * l_address  = new QLabel(i18n("E-mail &Address"),   this);
    QLabel * l_reply    = new QLabel(i18n("&Reply address"),    this);

    le_name_    = new QLineEdit(this);
    le_org_     = new QLineEdit(this);
    le_address_ = new QLineEdit(this);
    le_reply_   = new QLineEdit(this);

    l_name      ->setBuddy(le_name_);
    l_org       ->setBuddy(le_org_);
    l_address   ->setBuddy(le_address_);
    l_reply     ->setBuddy(le_reply_);

    layout->addWidget(l_name,       0, 0);
    layout->addWidget(l_org,        1, 0);
    layout->addWidget(l_address,    2, 0);
    layout->addWidget(l_reply,      3, 0);
    
    layout->addWidget(le_name_,     0, 1);
    layout->addWidget(le_org_,      1, 1);
    layout->addWidget(le_address_,  2, 1);
    layout->addWidget(le_reply_,    3, 1);
    
    EmpathAddressValidator * validator = new EmpathAddressValidator(this);
    
    le_address_ ->setValidator(validator);
    le_reply_   ->setValidator(validator);
    
    QObject::connect(le_name_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    QObject::connect(le_org_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));
    
    QObject::connect(le_address_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));
    
    QObject::connect(le_reply_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    le_name_->setFocus();

    struct passwd * p = getpwuid(getuid());

    QString hostName;
    
    struct utsname utsName;
    
    if (uname(&utsName) == 0)
        hostName = utsName.nodename;

    QString name    = QString().fromLatin1(p->pw_gecos);
    QString email   = QString().fromLatin1(p->pw_name) + "@" + hostName;
    
    KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);

    c.setGroup("UserInfo");

    le_name_    ->setText(c.readEntry("FullName", name));
    le_address_ ->setText(c.readEntry("EmailAddress", email));
    le_org_     ->setText(c.readEntry("Organization"));
    le_reply_   ->setText(c.readEntry("ReplyAddr"));

    le_name_->setFocus();
    
    _validate();
}

EmpathUserInfoPage::~EmpathUserInfoPage()
{
    // Empty
}

    void
EmpathUserInfoPage::saveConfig()
{
    KConfig * c;
    
    QString kcmemailrc = KGlobal::dirs()->findResource("config", "kcmemailrc");
    
    
    if (!kcmemailrc.isEmpty())
        c = new KConfig(KGlobal::dirs()->findResource("config", "kcmemailrc"));
    
    else {
        
        QString dir = *(KGlobal::dirs()->resourceDirs("config").at(0));
        c = new KConfig(dir + "kcmemailrc");
    }

    c->setGroup("UserInfo");

    c->writeEntry("FullName",       le_name_    ->text());
    c->writeEntry("Organization",   le_org_     ->text());
    c->writeEntry("EmailAddress",   le_address_ ->text());
    c->writeEntry("ReplyAddr",      le_reply_   ->text());

    c->sync();

    delete c;
    c = 0;
}
 
    QString
EmpathUserInfoPage::user()
{
    return le_name_->text();
}

    QString
EmpathUserInfoPage::org()
{
    return le_org_->text();
}

    QString
EmpathUserInfoPage::address()
{
    return le_address_->text();
}

    QString
EmpathUserInfoPage::reply()
{
    return le_reply_->text();
}

    void
EmpathUserInfoPage::s_textChanged(const QString &)
{
    _validate();
}

    void
EmpathUserInfoPage::_validate()
{
    int i(0);
    
    bool nameOK     = !le_name_->text().isEmpty();
    QString addr    = le_address_->text();
    QString reply   = le_reply_->text();
    bool noReply    = le_reply_->text().isEmpty();

    bool addressOK =
        (le_address_->validator()->validate(addr, i) == QValidator::Valid);
    
    bool replyOK =
        (le_reply_->validator()->validate(reply, i) == QValidator::Valid);
  
    bool allOK = nameOK && addressOK && (noReply || replyOK);

    emit(continueOK(this, allOK));
}

EmpathFolderInfoPage::EmpathFolderInfoPage(QWidget * parent)
    :   QWidget(parent, "FolderInfoPage")
{
    QGridLayout * layout = new QGridLayout(this, 5, 2, 10, 10); 
    
    QLabel * l_inbox    = new QLabel(i18n("New &mail"),             this);
    QLabel * l_outbox   = new QLabel(i18n("&Waiting to be sent"),   this);
    QLabel * l_drafts   = new QLabel(i18n("&Postponed (drafts)"),   this);
    QLabel * l_sent     = new QLabel(i18n("&Confirmed as sent"),    this);
    QLabel * l_trash    = new QLabel(i18n("&Unwanted"),             this);

    le_inbox_   = new QLineEdit(i18n("Inbox"),  this);
    le_outbox_  = new QLineEdit(i18n("Outbox"), this);
    le_drafts_  = new QLineEdit(i18n("Drafts"), this);
    le_sent_    = new QLineEdit(i18n("Sent"),   this);
    le_trash_   = new QLineEdit(i18n("Trash"),  this);

    l_inbox     ->setBuddy(le_inbox_);
    l_outbox    ->setBuddy(le_outbox_);
    l_drafts    ->setBuddy(le_drafts_);
    l_sent      ->setBuddy(le_sent_);
    l_trash     ->setBuddy(le_trash_);

    layout->addWidget(l_inbox,      0, 0);
    layout->addWidget(l_outbox,     1, 0);
    layout->addWidget(l_drafts,     2, 0);
    layout->addWidget(l_sent,       3, 0);
    layout->addWidget(l_trash,      4, 0);
    
    layout->addWidget(le_inbox_,    0, 1);
    layout->addWidget(le_outbox_,   1, 1);
    layout->addWidget(le_drafts_,   2, 1);
    layout->addWidget(le_sent_,     3, 1);
    layout->addWidget(le_trash_,    4, 1);
    
    layout->activate();
    
    QObject::connect(le_inbox_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    QObject::connect(le_outbox_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    QObject::connect(le_drafts_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    QObject::connect(le_sent_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    QObject::connect(le_trash_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    le_inbox_->setFocus();
    
    _validate();
}

EmpathFolderInfoPage::~EmpathFolderInfoPage()
{
    // Empty
}

    QString
EmpathFolderInfoPage::inbox()
{
    return le_inbox_->text();
}
    
    QString
EmpathFolderInfoPage::outbox()
{
    return le_outbox_->text();
}
    
    QString
EmpathFolderInfoPage::sent()
{
    return le_sent_->text();
}
    
    QString
EmpathFolderInfoPage::drafts()
{
    return le_drafts_->text();
}
    
    QString
EmpathFolderInfoPage::trash()
{
    return le_trash_->text();
}

    void
EmpathFolderInfoPage::saveConfig()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_FOLDERS);

    c->writeEntry(FOLDER_INBOX,     le_inbox_   ->text());
    c->writeEntry(FOLDER_OUTBOX,    le_outbox_  ->text());
    c->writeEntry(FOLDER_DRAFTS,    le_drafts_  ->text());
    c->writeEntry(FOLDER_SENT,      le_sent_    ->text());
    c->writeEntry(FOLDER_TRASH,     le_trash_   ->text());

    EmpathMailbox * _m =
        empath->mailboxList()->createNew(EmpathMailbox::Maildir);

    if (_m == 0) {
        empathDebug("Cannot create mailbox !!!!!");
        abort();
    }

    EmpathMailboxMaildir * m = (EmpathMailboxMaildir *)_m;

    m->setName(i18n("Local"));
    QString home = QString::fromLatin1(getenv("HOME"));
    empathDebug("Setting new maildir box path to " + home + "/.empath");
    m->setPath(home + "/.empath");
    empath->mailboxList()->saveConfig();
}
 
    void
EmpathFolderInfoPage::s_textChanged(const QString &)
{
    _validate();
}

    void
EmpathFolderInfoPage::_validate()
{
    bool allOK =
        !le_inbox_  ->text().isEmpty() &&
        !le_outbox_ ->text().isEmpty() &&
        !le_drafts_ ->text().isEmpty() &&
        !le_sent_   ->text().isEmpty() &&
        !le_trash_  ->text().isEmpty();

    emit(continueOK(this, allOK));
}

EmpathPOPInfoPage::EmpathPOPInfoPage(QWidget * parent)
    :   QWidget(parent, "POPInfoPage")
{
    QVBoxLayout * layout = new QVBoxLayout(this, 10);

    cb_use_ = new QCheckBox(i18n("Download mail from a POP3 server"), this);
    
    QObject::connect(cb_use_,
        SIGNAL(toggled(bool)),
        SLOT(s_enableWidgets(bool)));
    
    cb_use_->setChecked(false);
    
    QGridLayout * layout2 = new QGridLayout;

    l_server_   = new QLabel(i18n("POP3 &server"), this);
    l_port_     = new QLabel(i18n("Port n&umber"), this);
    l_user_     = new QLabel(i18n("&Username"), this);
    l_pass_     = new QLabel(i18n("&Password"), this);
    
    le_server_  = new QLineEdit(this);
    le_user_    = new QLineEdit(this);
    le_pass_    = new QLineEdit(this);
    sb_port_    = new QSpinBox(1, 100000, 1, this);
    sb_port_->setValue(110);
    
    l_server_   ->setBuddy(le_server_);
    l_port_     ->setBuddy(sb_port_);
    l_user_     ->setBuddy(le_user_);
    l_pass_     ->setBuddy(le_pass_);

    layout2->addWidget(l_server_,   0, 0);
    layout2->addWidget(l_port_,     1, 0);
    layout2->addWidget(l_user_,     2, 0);
    layout2->addWidget(l_pass_,     3, 0);
    
    layout2->addWidget(le_server_,  0, 1);
    layout2->addWidget(sb_port_,    1, 1);
    layout2->addWidget(le_user_,    2, 1);
    layout2->addWidget(le_pass_,    3, 1);

    layout->addWidget(cb_use_);
    layout->addLayout(layout2);
    
    cb_use_->setFocus();
    
    QObject::connect(le_server_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));
    
    QObject::connect(le_user_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));
    
    QObject::connect(le_pass_,
        SIGNAL(textChanged(const QString &)),
        SLOT(s_textChanged(const QString &)));

    s_enableWidgets(false);

    _validate();
}

EmpathPOPInfoPage::~EmpathPOPInfoPage()
{
    // Empty.
}

    void
EmpathPOPInfoPage::s_textChanged(const QString &)
{
    _validate();
}

    void
EmpathPOPInfoPage::s_enableWidgets(bool b)
{
    l_server_->setEnabled(b);
    l_port_->setEnabled(b);
    l_user_->setEnabled(b);
    l_pass_->setEnabled(b);

    le_server_->setEnabled(b);
    sb_port_->setEnabled(b);
    le_user_->setEnabled(b);
    le_pass_->setEnabled(b);

    _validate();
}

    bool
EmpathPOPInfoPage::enabled()
{
    return cb_use_->isChecked();
}

    QString
EmpathPOPInfoPage::server()
{
    return le_server_->text();
}

    unsigned int
EmpathPOPInfoPage::port()
{
    return sb_port_->value();
}

    QString
EmpathPOPInfoPage::username()
{
    return le_user_->text();
}

    QString
EmpathPOPInfoPage::password()
{
    return le_pass_->text();
}

    void
EmpathPOPInfoPage::saveConfig()
{
    if (!cb_use_->isChecked())
        return;

    EmpathMailbox * _m = empath->mailboxList()->createNew(EmpathMailbox::POP3);

    if (_m == 0) {
        empathDebug("Cannot create mailbox !!!!!");
        abort();
    }

    EmpathMailboxPOP3 * m = (EmpathMailboxPOP3 *)_m;

    m->setServerAddress (server());
    m->setServerPort    (port());
    m->setUsername      (username());
    m->setPassword      (password());

    m->init();

    empath->mailboxList()->saveConfig();
}

    void
EmpathPOPInfoPage::_validate()
{
    if (!cb_use_->isChecked()) {
        emit(continueOK(this, true));
        return;
    }

    bool allOK =
        !le_server_->text().isEmpty()   &&
        !le_user_->text().isEmpty()     &&
        !le_pass_->text().isEmpty();

    emit(continueOK(this, allOK));
}


EmpathIMAPInfoPage::EmpathIMAPInfoPage(QWidget * parent)
    :   QWidget(parent, "IMAPInfoPage")
{
}

EmpathIMAPInfoPage::~EmpathIMAPInfoPage()
{
}

EmpathReviewPage::EmpathReviewPage(QWidget * parent)
    :   QWidget(parent, "ReviewPage")
{
    QGridLayout * layout = new QGridLayout(this, 1, 1, 10, 10); 
    QLabel * l_review = new QLabel(i18n("Review your options"), this);
    layout->addWidget(l_review, 0, 0);
    layout->activate();

}

EmpathReviewPage::~EmpathReviewPage()
{
    // Empty
}

EmpathAddressValidator::EmpathAddressValidator(QWidget * parent)
    :   QValidator(parent, "AddressValidator")
{
    // Empty.
}

EmpathAddressValidator::~EmpathAddressValidator()
{
    // Empty.
}

    QValidator::State
EmpathAddressValidator::validate(QString & s, int &) const
{
    if (s.contains('@'))
        return QValidator::Valid;
    
    return QValidator::Acceptable;
}

// vim:ts=4:sw=4:tw=78
