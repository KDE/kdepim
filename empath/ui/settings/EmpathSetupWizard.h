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
# pragma interface "EmpathSetupWizard.h"
#endif

#ifndef EMPATHSETUPWIZARD_H
#define EMPATHSETUPWIZARD_H

// Qt includes
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvalidator.h>

// KDE includes
#include <kwizard.h>

class EmpathWelcomePage;
class EmpathUserInfoPage;
class EmpathFolderInfoPage;
class EmpathIMAPInfoPage;
class EmpathPOPInfoPage;
class EmpathReviewPage;

class EmpathSetupWizard : public KWizard
{
    Q_OBJECT

    public:
        
        EmpathSetupWizard();
        
        virtual ~EmpathSetupWizard();

        bool appropriate(QWidget *) const;
        
    protected slots:
        
        void s_continueOK(QWidget *, bool);
        void accept();

    private:

        EmpathWelcomePage       * welcomePage_;
        EmpathUserInfoPage      * userPage_;
        EmpathFolderInfoPage    * folderPage_;
        EmpathIMAPInfoPage      * imapPage_;
        EmpathPOPInfoPage       * popPage_;
        EmpathReviewPage        * reviewPage_;
};

class EmpathWelcomePage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathWelcomePage(QWidget *);
        virtual ~EmpathWelcomePage();
        
    private:
};


class EmpathUserInfoPage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathUserInfoPage(QWidget *);
        virtual ~EmpathUserInfoPage();
        
        QString user();
        QString org();
        QString address();
        QString reply();
        
        void saveConfig();

    protected slots:
        
        void s_textChanged(const QString &);
        
    signals:
    
        void continueOK(QWidget *, bool);
        
    private:

        void _validate();
        
        QLineEdit * le_name_;
        QLineEdit * le_org_;
        QLineEdit * le_address_;
        QLineEdit * le_reply_;
};

class EmpathFolderInfoPage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathFolderInfoPage(QWidget *);
        virtual ~EmpathFolderInfoPage();

        QString inbox();
        QString outbox();
        QString sent();
        QString drafts();
        QString trash();

        void saveConfig();
    
    protected slots:
        
        void s_textChanged(const QString &);

    signals:
    
        void continueOK(QWidget *, bool);

    private:
        
        void _validate();

        QLineEdit * le_inbox_;
        QLineEdit * le_outbox_;
        QLineEdit * le_drafts_;
        QLineEdit * le_sent_;
        QLineEdit * le_trash_;
};

class EmpathIMAPInfoPage : public QWidget
{
    Q_OBJECT
        
    public:

        EmpathIMAPInfoPage(QWidget *);
        virtual ~EmpathIMAPInfoPage();

        void saveConfig();
    
    signals:
    
        void continueOK(bool);
};


class EmpathPOPInfoPage : public QWidget
{
    Q_OBJECT
        
    public:

        EmpathPOPInfoPage(QWidget *);
        virtual ~EmpathPOPInfoPage();

        void saveConfig();

        bool enabled();
        QString server();
        unsigned int port(); 
        QString username();
        QString password();

    protected slots:

        void s_enableWidgets(bool);
        void s_textChanged(const QString &);
    
    signals:
    
        void continueOK(QWidget *, bool);

    private:
        
        void _validate();

        QLabel      * l_server_;
        QLabel      * l_port_;
        QLabel      * l_user_;
        QLabel      * l_pass_;

        QCheckBox   * cb_use_;
        QLineEdit   * le_server_;
        QSpinBox    * sb_port_;
        QLineEdit   * le_user_;
        QLineEdit   * le_pass_;
};

class EmpathReviewPage : public QWidget
{
    Q_OBJECT
        
    public:
        
        EmpathReviewPage(QWidget *);
        virtual ~EmpathReviewPage();
        
    private:
};

class EmpathAddressValidator : public QValidator
{
    Q_OBJECT
        
    public:
        
        EmpathAddressValidator(QWidget *);
        ~EmpathAddressValidator();
        QValidator::State validate(QString &, int &) const;
};


#endif

// vim:ts=4:sw=4:tw=78

