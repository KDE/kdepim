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
# pragma interface "EmpathConfigIMAP4Dialog.h"
#endif

#ifndef EMPATHCONFIGIMAP4DIALOG_H
#define EMPATHCONFIGIMAP4DIALOG_H

// Qt includes
#include <qwidget.h>
#include <qpushbutton.h>

// KDE includes
#include <kdialog.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathURL.h"

class EmpathMailboxIMAP4;

/**
 * Configure an imap mailbox
 */
class EmpathConfigIMAP4Dialog : public KDialog
{
    Q_OBJECT

    public:
        
        static void create(const EmpathURL &, QWidget * = 0);

        ~EmpathConfigIMAP4Dialog();

        void fillInSavedData();
        
        void closeEvent(QCloseEvent * e)
        { e->accept(); if (parent() == 0) delete this; }

    protected slots:

        void    s_OK();
        void    s_Cancel();
        void    s_Help();

    protected:

    private:

        EmpathConfigIMAP4Dialog(const EmpathURL &, QWidget *  = 0);
        
        void saveData();
        void loadData();

        EmpathURL url_;

        QPushButton * pb_OK_;
        QPushButton * pb_Cancel_;
        QPushButton * pb_Help_;
    
        static bool exists_;
};

#endif
// vim:ts=4:sw=4:tw=78
