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
# pragma implementation "EmpathSetupWizard.h"
#endif

// Qt includes
#include <qlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

// KDE includes
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathSetupWizard.h"

    void
EmpathSetupWizard::create()
{
    EmpathSetupWizard * wiz = new EmpathSetupWizard;
    wiz->show();
}

EmpathSetupWizard::~EmpathSetupWizard()
{
    // Empty.
}
        
EmpathSetupWizard::EmpathSetupWizard()
    :   QWizard(0, "SetupWizard")
{
    setCaption(i18n("Setup Wizard"));

    welcomePage_        = new EmpathWelcomePage(this);
    userPage_           = new EmpathUserInfoPage(this);
    accountTypePage_    = new EmpathAccountTypePage(this);
    accountSetupPage_   = new EmpathAccountSetupPage(this);
    reviewPage_         = new EmpathReviewPage(this);

    addPage(welcomePage_,       i18n("Welcome to Empath"));
    addPage(userPage_,          i18n("User Information"));
    addPage(accountTypePage_,   i18n("Account Type"));
    addPage(accountSetupPage_,  i18n("Account Setup"));
    addPage(reviewPage_,        i18n("Review Settings"));
    
    // FIXME: I don't understand what appropriate means really.
    setAppropriate(welcomePage_,        true);
    setAppropriate(userPage_,           true);
    setAppropriate(accountTypePage_,    true);
    setAppropriate(accountSetupPage_,   true);
    setAppropriate(reviewPage_,         true);
    
    // XXX: I do understand this bit though.
    setNextEnabled(userPage_,           false);
    setNextEnabled(accountSetupPage_,   false);
    
    QObject::connect(userPage_, SIGNAL(continueOK(bool)),
        this, SLOT(s_userContinueOK(bool)));

    QObject::connect(accountSetupPage_, SIGNAL(continueOK(bool)),
        this, SLOT(s_accountSetupContinueOK(bool)));
}

    void
EmpathSetupWizard::s_setPop(bool b)
{
    if (b)
        accountSetupPage_->setPop();
}

    void
EmpathSetupWizard::s_setLocal(bool b)
{
    if (b)
        accountSetupPage_->setLocal();
}

    void
EmpathSetupWizard::s_setHelp(bool b)
{
    if (b)
        accountSetupPage_->setHelp();
}

    void
EmpathSetupWizard::s_userContinueOK(bool b)
{
    setNextEnabled(userPage_, b);
}

    void
EmpathSetupWizard::s_accountSetupContinueOK(bool b)
{
    setNextEnabled(accountSetupPage_, b);
}

EmpathWelcomePage::EmpathWelcomePage(QWidget * parent, const char *)
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

EmpathUserInfoPage::EmpathUserInfoPage(QWidget * parent, const char *)
    :   QWidget(parent, "UserInfoPage")
{
    QGridLayout * layout = new QGridLayout(this, 2, 2, 10, 10); 
    
    QLabel * l_name = new QLabel(i18n("Name"), this);
    layout->addWidget(l_name, 0, 0);
    
    QLabel * l_address = new QLabel(i18n("Email Address"), this);
    layout->addWidget(l_address, 1, 0);
    
    le_name_ = new QLineEdit(this);
    layout->addWidget(le_name_, 0, 1);
    
    le_address_ = new QLineEdit(this);
    layout->addWidget(le_address_, 1, 1);
    
    le_address_->setValidator(new EmpathAddressValidator(le_address_));
    
    layout->activate();
    
    QObject::connect(le_name_, SIGNAL(textChanged(const QString &)),
        this, SLOT(s_textChanged(const QString &)));

    QObject::connect(le_address_, SIGNAL(textChanged(const QString &)),
        this, SLOT(s_textChanged(const QString &)));

    le_name_->setFocus();
}

EmpathUserInfoPage::~EmpathUserInfoPage()
{
    // Empty
}

    void
EmpathUserInfoPage::s_textChanged(const QString &)
{
    bool nameOK = !le_name_->text().isEmpty();

    int i(0);
    QString address(le_address_->text());
    bool addressOK =
    (le_address_->validator()->validate(address, i) == QValidator::Acceptable);

    emit(continueOK(nameOK && addressOK));
}

EmpathAccountTypePage::EmpathAccountTypePage(QWidget * parent, const char *)
    :   QWidget(parent, "AccountTypePage")
{
    QGridLayout * layout = new QGridLayout(this, 5, 1, 0, 0);

    QButtonGroup * group = new QButtonGroup(this);
    group->hide();
    
    QLabel * method_= new QLabel(
        i18n("Please specify what kind of mailbox you use.\n"), this);
    
    QRadioButton * pop_ = new QRadioButton(
        i18n("POP - I download my mail"), this);
    
    QRadioButton * local_ = new QRadioButton(
        i18n("Local - My mail is automatically delivered to me"), this);
    
    QRadioButton * dunno_ = new QRadioButton(
        i18n("I don't know - help !"), this);
    
    group->insert(pop_,     0);
    group->insert(local_,   1);
    group->insert(dunno_,   2);
    
    group->setButton(0); // Default to POP.
    pop_->setFocus();
    
    layout->addWidget(method_, 0, 0);
    layout->addWidget(pop_,    1, 0);
    layout->addWidget(local_,  2, 0);
    layout->addWidget(dunno_,  3, 0);
    layout->activate();

    QObject::connect(
        pop_,   SIGNAL(toggled(bool)),
        parent, SLOT(s_setPop(bool)));

    QObject::connect(
        local_, SIGNAL(toggled(bool)),
        parent, SLOT(s_setLocal(bool)));

    QObject::connect(
        dunno_, SIGNAL(toggled(bool)),
        parent, SLOT(s_setHelp(bool)));
}

EmpathAccountTypePage::~EmpathAccountTypePage()
{
    // Empty
}

EmpathAccountSetupPage::EmpathAccountSetupPage(QWidget * parent, const char *)
    :   QWidget(parent, "AccountSetupPage")
{
}

    void
EmpathAccountSetupPage::setupPop()
{
    QLabel * l_hostname = new QLabel(i18n("Server name"), this);
    QLabel * l_port     = new QLabel(i18n("Port"), this);
    QLabel * l_username = new QLabel(i18n("Username"), this);
    QLabel * l_password = new QLabel(i18n("Password"), this);
    
    le_hostname_    = new QLineEdit(this);
    sb_port_        = new QSpinBox(this);
    le_username_    = new QLineEdit(this);
    le_password_    = new QLineEdit(this);
 
    QGridLayout * layout = new QGridLayout(this, 4, 2, 10, 10); 

    layout->addWidget(l_hostname,   0, 0);
    layout->addWidget(l_port,       1, 0);
    layout->addWidget(l_username,   2, 0);
    layout->addWidget(l_password,   3, 0);

    layout->addWidget(le_hostname_, 0, 1);
    layout->addWidget(sb_port_,     1, 1);
    layout->addWidget(le_username_, 2, 1);
    layout->addWidget(le_password_, 3, 1);
       
    layout->activate();
    
    le_hostname_->setFocus();
}

    void
EmpathAccountSetupPage::setupLocal()
{
    QLabel * l_hostname = new QLabel(i18n("Server name"), this);
    QLabel * l_port     = new QLabel(i18n(""), this);
    
    le_hostname_    = new QLineEdit(this);
    sb_port_        = new QSpinBox(this);
    le_username_    = new QLineEdit(this);
    le_password_    = new QLineEdit(this);
 
    QGridLayout * layout = new QGridLayout(this, 4, 2, 10, 10); 

    layout->addWidget(l_hostname,   0, 0);
    layout->addWidget(l_port,       1, 0);
    layout->addWidget(l_username,   2, 0);
    layout->addWidget(l_password,   3, 0);

    layout->addWidget(le_hostname_, 0, 1);
    layout->addWidget(sb_port_,     1, 1);
    layout->addWidget(le_username_, 2, 1);
    layout->addWidget(le_password_, 3, 1);
       
    layout->activate();
    
    le_hostname_->setFocus();

}

    void
EmpathAccountSetupPage::setupHelp()
{
}

EmpathAccountSetupPage::~EmpathAccountSetupPage()
{
    // Empty
}

EmpathReviewPage::EmpathReviewPage(QWidget * parent, const char *)
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

// vim:ts=4:sw=4:tw=78
