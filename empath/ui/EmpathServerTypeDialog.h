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
# pragma interface "EmpathServerTypeDialog.h"
#endif

#ifndef EMPATHSERVERTYPEDIALOG_H
#define EMPATHSERVERTYPEDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathMailbox.h"
#include "RikGroupBox.h"

class EmpathServerTypeDialog : public QDialog
{
    Q_OBJECT

    public:
        
        EmpathServerTypeDialog(QWidget * parent = 0, const char * name = 0);

        ~EmpathServerTypeDialog() { empathDebug("dtor"); }

        EmpathMailbox::AccountType accountType();

    protected slots:

        void s_OK();
        void s_Cancel();
        void s_Help();

    private:

        RikGroupBox        * rgb_type_;
        QButtonGroup    * buttonGroup_;
    
        QWidget            * w_type_;
    
        QPushButton        * pb_OK_;
        QPushButton        * pb_Cancel_;
        QPushButton        * pb_Help_;
        
        QRadioButton    * rb_serverTypeMaildir_;
        QRadioButton    * rb_serverTypePOP3_;
        QRadioButton    * rb_serverTypeIMAP4_;
    
        QGridLayout        * topLevelLayout_;
        QGridLayout        * typeGroupLayout_;

        KButtonBox        * buttonBox_;
};

#endif
// vim:ts=4:sw=4:tw=78
