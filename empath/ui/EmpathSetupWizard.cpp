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
#include "EmpathAccountsSettingsWidget.h"

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

    welcomePage_    = new EmpathWelcomePage(this);
    userPage_       = new EmpathUserInfoPage(this);
    accountPage_    = new EmpathAccountInfoPage(this);
    reviewPage_     = new EmpathReviewPage(this);

    addPage(welcomePage_,   i18n("Welcome to Empath"));
    addPage(userPage_,      i18n("User Information"));
    addPage(accountPage_,   i18n("Account Information"));
    addPage(reviewPage_,    i18n("Review Settings"));
    
    setAppropriate(welcomePage_,    true);
    setAppropriate(userPage_,       true);
    setAppropriate(accountPage_,    false);
    setAppropriate(reviewPage_,     false);
    
    setNextEnabled(userPage_,       false);
    setNextEnabled(accountPage_,    false);
    
    QObject::connect(userPage_, SIGNAL(continueOK(bool)),
        this, SLOT(s_userContinueOK(bool)));

    QObject::connect(accountPage_, SIGNAL(continueOK(bool)),
        this, SLOT(s_accountContinueOK(bool)));
}

    void
EmpathSetupWizard::s_userContinueOK(bool b)
{
    setAppropriate(accountPage_, b);
    setNextEnabled(userPage_, b);
}

    void
EmpathSetupWizard::s_accountContinueOK(bool b)
{
    empathDebug(b ? "true" : "false");
    setAppropriate(reviewPage_, b);
    setNextEnabled(accountPage_, b);
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

EmpathAccountInfoPage::EmpathAccountInfoPage(QWidget * parent, const char *)
    :   QWidget(parent, "AccountInfoPage")
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
    
    emit(continueOK(true));
}

EmpathAccountInfoPage::~EmpathAccountInfoPage()
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
