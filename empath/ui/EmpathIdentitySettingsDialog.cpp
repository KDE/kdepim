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
# pragma implementation "EmpathIdentitySettingsDialog.h"
#endif

// System includes
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

// Qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qfont.h>

// KDE includes
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kfiledialog.h>

// Local includes
#include "EmpathIdentitySettingsDialog.h"
#include "EmpathPathSelectWidget.h"
#include "EmpathSeparatorWidget.h"
#include "EmpathConfig.h"
#include "EmpathUtilities.h"
#include "EmpathUIUtils.h"
#include "Empath.h"
        
EmpathIdentitySettingsDialog::EmpathIdentitySettingsDialog(QWidget * parent)
    :   KDialog(parent, "IdentitySettings", true),
        applied_(false)
{
    setCaption(i18n("Identity Settings"));
    

    QLabel * l_name     = new QLabel(i18n("Full name"),     this, "l_name");
    QLabel * l_email    = new QLabel(i18n("Email address"), this, "l_email");
    QLabel * l_replyTo  = new QLabel(i18n("Reply address"), this, "l_replyTo");
    QLabel * l_org      = new QLabel(i18n("Organization"),  this, "l_org");
    QLabel * l_sig      = new QLabel(i18n("Signature file"),this, "l_sig");
    
    le_name_      = new QLineEdit(this, "le_Name");
    le_email_     = new QLineEdit(this, "le_Email");
    le_replyTo_   = new QLineEdit(this, "le_ReplyTo");
    le_org_       = new QLineEdit(this, "le_Org");
    efsw_sig_     = new EmpathFileSelectWidget(QString::null, this);
    
    pb_editSig_ = new QPushButton(i18n("Edit"), this, "pb_editSig");

    mle_sig_ = new QMultiLineEdit(this, "mle_sig");

    mle_sig_->setReadOnly(true);
    mle_sig_->setFont(KGlobal::fixedFont());
    mle_sig_->setText(i18n("No signature set"));
    mle_sig_->setMinimumHeight(60);
    
#include "EmpathDialogButtonBox.cpp"
    
    // Layout
    
    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QGridLayout * layout0 = new QGridLayout(layout);
    
    layout0->addWidget(l_name,      0, 0);
    layout0->addWidget(l_email,     1, 0);
    layout0->addWidget(l_replyTo,   2, 0);
    layout0->addWidget(l_org,       3, 0);

    layout0->addWidget(le_name_,      0, 1);
    layout0->addWidget(le_email_,     1, 1);
    layout0->addWidget(le_replyTo_,   2, 1);
    layout0->addWidget(le_org_,       3, 1);

    layout->addWidget(new EmpathSeparatorWidget(this));
    
    QHBoxLayout * layout1 = new QHBoxLayout;
    
    layout1->addWidget(l_sig);
    layout1->addWidget(efsw_sig_);
    layout1->addWidget(pb_editSig_);

    layout->addLayout(layout1);
    layout->addWidget(mle_sig_);
    
    layout->addSpacing(10);
 
    layout->addWidget(new EmpathSeparatorWidget(this));
   
    layout->addWidget(buttonBox_);
 
    QString helpEmail = i18n(
        "This should contain your email address.\n"
        "Type it correctly !");

    QString helpName = i18n("This should contain your real name");

    QString helpReplyTo = i18n(
        "If your 'real' email address isn't the\n"
        "address you want people to reply to,\n"
        "then simply fill this in.");

    QString helpOrg = i18n(
        "This is supposed to contain the name of\n"
        "the organization you belong to. You can\n"
        "type anything, or nothing. It doesn't\n"
        "matter in the slightest.");

    QString helpSig = i18n(
        "The name of a file containing your 'signature'.\n"
        "Your signature can be appended to the end of\n"
        "each message you send. People generally like to\n"
        "put some info about where they work, a quote,\n"
        "and/or their website address here. Please try\n"
        "to use less than four lines. It's netiquette.\n");

    QWhatsThis::add(l_email,    helpEmail);
    QWhatsThis::add(l_name,     helpName);
    QWhatsThis::add(l_replyTo,  helpReplyTo);
    QWhatsThis::add(l_org,      helpOrg);
    QWhatsThis::add(l_sig,      helpSig);
 
    QWhatsThis::add(le_email_,    helpEmail);
    QWhatsThis::add(le_name_,     helpName);
    QWhatsThis::add(le_replyTo_,  helpReplyTo);
    QWhatsThis::add(le_org_,      helpOrg);
    QWhatsThis::add(efsw_sig_,    helpSig);
   
    QWhatsThis::add(pb_editSig_, i18n(
        "Press this to edit your signature.\n"
        "Note that you just use the box below !\n"
        "When you're done, press the button again\n"
        "to save your signature."));
    
    QObject::connect(
        efsw_sig_,  SIGNAL(changed(const QString &)),
        this,       SLOT(s_sigChanged(const QString &)));

    QObject::connect(pb_editSig_,   SIGNAL(clicked()),  SLOT(s_editSig()));
}

EmpathIdentitySettingsDialog::~EmpathIdentitySettingsDialog()
{
    // Empty.
}

    void
EmpathIdentitySettingsDialog::s_sigChanged(const QString & newPath)
{
    // Preview the sig
    QFile f(newPath);
    if (!f.open(IO_ReadOnly)) return;
    
    QTextStream t(&f);
    QString s;
    QString buf;

    while (!t.eof())
        s += t.readLine() + "\n";
    
    f.close();
    mle_sig_->setText(s);
}

    void
EmpathIdentitySettingsDialog::saveData()
{
    KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);

    c.setGroup("UserInfo");

    c.writeEntry("FullName",        le_name_->text());
    c.writeEntry("EmailAddress",    le_email_->text());
    c.writeEntry("Organization",    le_org_->text());
    c.writeEntry("ReplyAddr",       le_replyTo_->text());

    KConfig * ec(KGlobal::config());
    
    using namespace EmpathConfig;
    
    ec->setGroup(GROUP_IDENTITY);
    ec->writeEntry(C_SIG_PATH, efsw_sig_->path());
}

    void
EmpathIdentitySettingsDialog::loadData()
{
    struct passwd * p = getpwuid(getuid());

    QString hostName;
    
    struct utsname utsName;
    
    if (uname(&utsName) == 0)
        hostName = utsName.nodename;

    QString name    = QString().fromLatin1(p->pw_gecos);
    QString email   = QString().fromLatin1(p->pw_name) + "@" + hostName;
    
    KConfig c(KGlobal::dirs()->findResource("config", "kcmemailrc"), true);

    c.setGroup("UserInfo");

    le_name_      ->setText(c.readEntry("FullName", name));
    le_email_     ->setText(c.readEntry("EmailAddress", email));
    le_org_       ->setText(c.readEntry("Organization"));
    le_replyTo_   ->setText(c.readEntry("ReplyAddr"));

    KConfig * ec(KGlobal::config());

    using namespace EmpathConfig;

    ec->setGroup(GROUP_IDENTITY);
    
    efsw_sig_->setPath(ec->readEntry(C_SIG_PATH));

    if (!QString(efsw_sig_->path()).isEmpty()) {
        
        // Preview the sig
        
        QFile f(efsw_sig_->path());

        if (f.open(IO_ReadOnly)) {

            mle_sig_->setReadOnly(false);
            mle_sig_->clear();

            QTextStream t(&f);

            QString s;

            while (!t.eof())
                s = s + t.readLine() + '\n';
            
            s.remove(s.length() - 1, 1);

            f.close();

            mle_sig_->setText(s);
            mle_sig_->setReadOnly(true);
        }
    }
}

    void
EmpathIdentitySettingsDialog::s_editSig()
{
    QObject::disconnect(pb_editSig_, SIGNAL(clicked()),
            this, SLOT(s_editSig()));

    mle_sig_->setReadOnly(false);

    pb_editSig_->setText(i18n("Save"));

    QObject::connect(pb_editSig_, SIGNAL(clicked()),
            this, SLOT(s_saveSig()));
}

    void
EmpathIdentitySettingsDialog::s_saveSig()
{
    QObject::disconnect(pb_editSig_, SIGNAL(clicked()),
            this, SLOT(s_saveSig()));

    mle_sig_->setReadOnly(true);

    QFile f(efsw_sig_->path());

    if (!f.open(IO_WriteOnly)) return;

    QTextStream t(&f);

    t << mle_sig_->text();

    pb_editSig_->setText(i18n("Edit"));

    QObject::connect(pb_editSig_, SIGNAL(clicked()),
            this, SLOT(s_editSig()));
}

    void
EmpathIdentitySettingsDialog::s_OK()
{
    if (!applied_)
        s_apply();
    KGlobal::config()->sync();
    accept();
}

    void
EmpathIdentitySettingsDialog::s_help()
{
    // STUB
}

    void
EmpathIdentitySettingsDialog::s_apply()
{
    if (applied_) {
        pb_apply_->setText(i18n("&Apply"));
        KGlobal::config()->rollback(true);
        KGlobal::config()->reparseConfiguration();
        loadData();
        applied_ = false;
    } else {
        pb_apply_->setText(i18n("&Revert"));
        pb_cancel_->setText(i18n("&Close"));
        applied_ = true;
    }
    saveData();
}

    void
EmpathIdentitySettingsDialog::s_default()
{
    struct passwd * p = getpwuid(getuid());

    QString hostName;
    
    struct utsname utsName;
    
    if (uname(&utsName) == 0)
        hostName = utsName.nodename;

    QString name    = QString().fromLatin1(p->pw_gecos);
    QString email   = QString().fromLatin1(p->pw_name) + "@" + hostName;
    
    le_name_    ->setText(name);
    le_email_   ->setText(email);
    le_replyTo_ ->setText(QString::null);
    efsw_sig_   ->setPath(QString::null);
    mle_sig_    ->setText(QString::null);

    saveData();
}
    
    void
EmpathIdentitySettingsDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
}

// vim:ts=4:sw=4:tw=78
