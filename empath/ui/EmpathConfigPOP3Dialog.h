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
# pragma interface "EmpathConfigPOP3Dialog.h"
#endif

#ifndef EMPATHCONFIGPOP3DIALOG_H
#define EMPATHCONFIGPOP3DIALOG_H

// Qt includes
#include <qdialog.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlabel.h>

// Local includes
#include "EmpathURL.h"

class EmpathPasswordEditWidget;

/**
 * Configure a pop3 mailbox.
 */
class EmpathConfigPOP3Dialog : public QDialog
{
    Q_OBJECT

    public:
        
        EmpathConfigPOP3Dialog(const EmpathURL &, QWidget * = 0);
        ~EmpathConfigPOP3Dialog();
        
    protected slots:

        void    s_OK();
        void    s_Cancel();
        void    s_Help();

    private:

        void saveData();
        void loadData();

        EmpathURL url_;

        QPushButton * pb_OK_;
        QPushButton * pb_Cancel_;
        QPushButton * pb_Help_;

        QLineEdit   * le_uname_;
        QLineEdit   * le_inServer_;
        QSpinBox    * sb_inServerPort_;
        
        EmpathPasswordEditWidget * epew_pass_;
};

#endif

// vim:ts=4:sw=4:tw=78
