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
# pragma interface "EmpathFilterEditDialog.h"
#endif

#ifndef EMPATHFILTEREDITDIALOG_H
#define EMPATHFILTEREDITDIALOG_H

// Qt includes
#include <qdialog.h>
#include <qwidget.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>

// KDE includes
#include <kbuttonbox.h>

class EmpathFilter;
class RikGroupBox;
class EmpathFolderChooserWidget;

class EmpathFilterEditDialog : public QDialog
{
    Q_OBJECT

    public:
    
        EmpathFilterEditDialog(
                EmpathFilter * filter,
                QWidget * parent = 0,
                const char * name = 0);
        
        virtual ~EmpathFilterEditDialog();
        
    protected slots:
        
        void s_OK();
        void s_cancel();
        void s_help();

        void s_addExpr();
        void s_editExpr();
        void s_removeExpr();
        void s_editAction();
        
    private:

        void            update();
        RikGroupBox        * rgb_arrives_;
        RikGroupBox        * rgb_matches_;
        RikGroupBox        * rgb_action_;

        QWidget            * w_arrives_;
        QWidget            * w_matches_;
        QWidget            * w_action_;

        QGridLayout        * mainLayout_;
        QGridLayout        * nameLayout_;
        QGridLayout        * arrivesLayout_;
        QGridLayout        * matchesLayout_;
        QGridLayout        * actionLayout_;

        QListBox        * lb_matches_;

        QLabel            * l_action_;
        
        QPushButton        * pb_addMatch_;
        QPushButton        * pb_editMatch_;
        QPushButton        * pb_removeMatch_;
    
        QPushButton        * pb_editAction_;

        QPushButton        * pb_OK_;
        QPushButton        * pb_cancel_;
        QPushButton        * pb_help_;

        KButtonBox        * buttonBox_;
        KButtonBox        * exprButtonBox_;

        QLabel            * l_arrivesFolder_;

        EmpathFolderChooserWidget * fcw_arrivesFolder_;

        EmpathFilter    * filter_;
        
        QLabel            * l_name_;
        QLineEdit        * le_name_;
};

#endif

// vim:ts=4:sw=4:tw=78
