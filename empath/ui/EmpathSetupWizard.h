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
# pragma interface "EmpathSetupWizard.h"
#endif

#ifndef EMPATHSETUPWIZARD_H
#define EMPATHSETUPWIZARD_H

// Qt includes
#include <qwizard.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qvalidator.h>

class EmpathWelcomePage;
class EmpathUserInfoPage;
class EmpathAccountTypePage;
class EmpathAccountSetupPage;
class EmpathReviewPage;

class EmpathSetupWizard : public QWizard
{
    Q_OBJECT

    public:
        
        static void create();

        virtual ~EmpathSetupWizard();
        
    protected slots:
        
        void s_userContinueOK(bool);
        void s_accountSetupContinueOK(bool);
        void s_setPop   (bool);
        void s_setLocal (bool);
        void s_setHelp  (bool);

    private:
        
        EmpathSetupWizard();

        EmpathWelcomePage       * welcomePage_;
        EmpathUserInfoPage      * userPage_;
        EmpathAccountTypePage   * accountTypePage_;
        EmpathAccountSetupPage  * accountSetupPage_;
        EmpathReviewPage        * reviewPage_;
};

class EmpathWelcomePage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathWelcomePage(QWidget * parent, const char * name = 0);
        virtual ~EmpathWelcomePage();
        
    private:
};


class EmpathUserInfoPage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathUserInfoPage(QWidget * parent, const char * name = 0);
        virtual ~EmpathUserInfoPage();
        
        QString userName() { return le_name_->text(); }
        QString address() { return le_address_->text(); }
        
    protected slots:
        
        void s_textChanged(const QString &);
        
    signals:
    
        void continueOK(bool);
        
    private:
        
        QLineEdit * le_name_, * le_address_;
};

class EmpathAccountTypePage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathAccountTypePage(QWidget * parent, const char * name = 0);
        virtual ~EmpathAccountTypePage();
};

class EmpathAccountSetupPage : public QWidget
{
    Q_OBJECT
        
    public:

        enum AccountType {
          POP,
          Local,
          None
        };
        
        EmpathAccountSetupPage(QWidget * parent, const char * name = 0);
        virtual ~EmpathAccountSetupPage();
        
        void showPop();
        void showLocal();
        void showHelp();

    
    signals:
    
        void continueOK(bool);
      
    private:

        // Widgets for POP setup.
        QLineEdit * le_hostname_;
        QLineEdit * le_username_;
        QLineEdit * le_password_;
        QSpinBox  * sb_port_;
};

class EmpathReviewPage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathReviewPage(QWidget * parent, const char * name = 0);
        virtual ~EmpathReviewPage();
        
    private:
};

class EmpathAddressValidator : public QValidator
{
    Q_OBJECT
        
    public:
        
        EmpathAddressValidator(QWidget * parent, const char * name = 0)
            :   QValidator(parent, name)
        {
            // Empty.
        }

        QValidator::State validate(QString & s, int &) const
        {
            if (s.isEmpty())
                return QValidator::Invalid;

            if (s.contains('@'))
                return QValidator::Acceptable;
            
            return QValidator::Valid;
        }
};


#endif

// vim:ts=4:sw=4:tw=78

