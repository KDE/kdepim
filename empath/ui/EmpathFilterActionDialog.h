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
# pragma interface "EmpathFilterActionDialog.h"
#endif

#ifndef EMPATHFILTERACTIONDIALOG_H
#define EMPATHFILTERACTIONDIALOG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

// KDE includes
#include <kbuttonbox.h>

class EmpathFilter;
class EmpathFolderChooserWidget;
class EmpathAddressSelectionWidget;

class EmpathFilterActionDialog : public QDialog
{
    Q_OBJECT

    public:

        EmpathFilterActionDialog(EmpathFilter * filter,
                QWidget * parent = 0, const char * name = 0);

        ~EmpathFilterActionDialog();

        void load();
        
    protected slots:

        void s_OK();
        void s_cancel();
        void s_help();

    private:

        QButtonGroup * bg_choices_;

        KButtonBox * buttonBox_;

        QPushButton * pb_OK_;
        QPushButton * pb_cancel_;
        QPushButton * pb_help_;

        QCheckBox * cb_continue_;

        QRadioButton * rb_moveFolder_;
        QRadioButton * rb_copyFolder_;
        QRadioButton * rb_delete_;
        QRadioButton * rb_ignore_;
        QRadioButton * rb_forwardTo_;

        EmpathFolderChooserWidget * fcw_moveFolder_;
        EmpathFolderChooserWidget * fcw_copyFolder_;

        EmpathAddressSelectionWidget * asw_address_;

        EmpathFilter * filter_;
};

#endif

// vim:ts=4:sw=4:tw=78
