
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
# pragma interface "EmpathFilterManagerDialog.h"
#endif

#ifndef EMPATHFILTERMANAGERDIALOG_H
#define EMPATHFILTERMANAGERDIALOG_H

// Qt includes
#include <qwidget.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdialog.h>

// KDE includes
#include <kbuttonbox.h>

// Local includes
#include "EmpathFilterListItem.h"
#include "EmpathDefines.h"

class RikGroupBox;

class EmpathFilterManagerDialog : public QDialog
{
    Q_OBJECT

    public:
    
        static void create();
        ~EmpathFilterManagerDialog();
        void saveData();
        void closeEvent(QCloseEvent * e) { e->accept(); delete this; }
        
    protected slots:
        
        void s_addFilter();
        void s_editFilter();
        void s_removeFilter();
        void s_moveUp();
        void s_moveDown();
        
        void s_OK();
        void s_cancel();
        void s_help();
        void s_apply();
        
    private:

        EmpathFilterManagerDialog(QWidget * parent = 0, const char * name = 0);

        void            update();

        RikGroupBox        * rgb_filters_;

        QWidget            * w_filters_;

        QGridLayout        * mainLayout_;
        QGridLayout        * filtersLayout_;

        QListView        * lv_filters_;

        QLabel            * l_about_;
        
        QPushButton        * pb_addFilter_;
        QPushButton        * pb_editFilter_;
        QPushButton        * pb_removeFilter_;
        QPushButton        * pb_moveUp_;
        QPushButton        * pb_moveDown_;
        QPushButton        * pb_editAction_;

        KButtonBox        * filtersButtonBox_;

        QList<EmpathFilterListItem> filterList_;

        KButtonBox        * buttonBox_;
        QPushButton        * pb_help_;
        QPushButton        * pb_apply_;
        QPushButton        * pb_OK_;
        QPushButton        * pb_cancel_;
        
        static bool        exists_;
        bool            applied_;
};

#endif

// vim:ts=4:sw=4:tw=78
